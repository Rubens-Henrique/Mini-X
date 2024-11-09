#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/ip.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <utils.h>
#include <sys/select.h>

int contador_cliente = 0;

struct
{
    int id;
    int fd;
} objeto_cliente[20];

int aceitar_conexao(int sockfd)
{
    struct sockaddr addr;
    socklen_t length = 0;

    int ret = accept(sockfd, &addr, &length);

    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Failed to accept a connection. errno: %i \n", errno);
    }

    return ret;
}

int definir_socket_addr(int sockfd, int port)
{

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(sockfd, (struct sockaddr *)(&addr), sizeof(struct sockaddr));

    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Falha ao vincular endereço: %i \n", errno);
    }

    return ret;
}

int escutar_socket(int sockfd)
{

    int ret = listen(sockfd, FILA_MAXIMA);

    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Falha ao definir socket para escutar: %i \n", errno);
    }

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getsockname(sockfd, (struct sockaddr *)&addr, &len) < 0)
    {
        fprintf(stderr, "\n ! ERRO: Falha ao pegar porta do socket: %i \n", errno);
    }
    else
    {
        fprintf(stderr, "\n Escutando em: %i\n", addr.sin_port);
    }

    return ret;
}

int obter_porta(int argc, char *argv[], int *port)
{
    if (argc != 2)
    {
        fprintf(stderr, "\n ! ERRO: Faltando argumentos.\n");
        HELPSERVER(argv[0]);
        return -1;
    }

    *port = atoi(argv[1]);
    if (*port < 1024 || *port > 65536)
    {
        fprintf(stderr, "\n ! ERRO: Porta está fora dos limites.\n");
        HELPSERVER(argv[0]);
        return -1;
    }
    return 0;
}

int servico_oi(struct msg_t msg, int sockfd)
{

    int id = msg.orig_uid;

    for (int i = 0; i < 20; i++)
    {
        if (objeto_cliente[i].id == id || objeto_cliente[i].id == (id + 1000))
        {
            fprintf(stderr, "\n ! ERRO: O ID já está em uso.\n");
            enviar_mensagem(sockfd, msg);
            return -1;
        }
    }

    msg.orig_uid = msg.dest_uid;
    msg.dest_uid = id;

    if (id > 0 && id < 1000)
    {
        for (int i = 0; i < 10; i++)
        {
            if (objeto_cliente[i].id == -1)
            {
                objeto_cliente[i].id = id;
                objeto_cliente[i].fd = sockfd;

                if (enviar_mensagem(sockfd, msg) == -1)
                {
                    return -1;
                }
                else
                {
                    contador_cliente += 1;
                    return 0;
                }
            }
        }
        fprintf(stderr, "\n ! ERRO: Servidor cheio.\n");
        return -1;
    }

    if (id > 1000 && id < 2000)
    {
        for (int i = 10; i < 20; i++)
        {
            if (objeto_cliente[i].id == -1)
            {
                objeto_cliente[i].id = id;
                objeto_cliente[i].fd = sockfd;
                if (enviar_mensagem(sockfd, msg) == -1)
                {
                    return -1;
                }
                else
                {
                    contador_cliente += 1;
                    return 0;
                }
            }
        }
        fprintf(stderr, "\n ! ERRO: Servidor cheio.\n");
        return -1;
    }

    fprintf(stderr, "\n ! ERRO: ID incorreto.\n");

    return -1;
}

int servico_tchau(int sockfd)
{
    for (int i = 0; i < 20; i++)
    {
        if (objeto_cliente[i].fd == sockfd)
        {
            objeto_cliente[i].id = -1;
            objeto_cliente[i].fd = -1;
            contador_cliente -= 1;
        }
    }

    struct msg_t tchau_msg;
    tchau_msg.type = TCHAU;
    tchau_msg.orig_uid = 0;
    tchau_msg.dest_uid = -1;
    tchau_msg.text_len = 0;

    enviar_mensagem(sockfd, tchau_msg);

    return 0;
}

int servico_tchau(struct msg_t msg)
{

    int orig = msg.orig_uid;
    int dest = msg.dest_uid;

    if ((dest > 1000) || (dest < 0))
    {
        fprintf(stderr, "\n ! ERRO: O Cliente de destino não é do tipo leitor.\n");
        return -1;
    }
    int aux = -1;
    for (int i = 10; i < 20; i++)
    {
        if (objeto_cliente[i].id == orig)
        {
            aux = 0;
            break;
        }
    }
    if (aux == -1)
    {
        fprintf(stderr, "\n ! ERRO: Remetente não cadastrado.\n");
        return -1;
    }

    if (dest > 0 && dest < 1000)
    {
        int dest_fd;
        aux = -1;
        for (int i = 0; i < 10; i++)
        {
            if (objeto_cliente[i].id == msg.dest_uid)
            {
                dest_fd = objeto_cliente[i].fd;
                aux = 0;
                break;
            }
        }
        if (aux == -1)
        {
            fprintf(stderr, "\n ! ERRO: Destinatário não encontrado.\n");
            return -1;
        }

        int ret = enviar_mensagem(dest_fd, msg);
        if (ret < 0)
        {
            return ret;
        }

        return 0;
    }

    if (dest == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            if (objeto_cliente[i].id > 0)
            {
                int ret = enviar_mensagem(objeto_cliente[i].fd, msg);
                if (ret < 0)
                {
                    return ret;
                }
            };
        }
        return 0;
    }

    fprintf(stderr, "\n ! ERRO !.\n");

    return -1;
}

time_t verificar_tempo(time_t *fim)
{
    time_t current_time;

    current_time = time(NULL);

    return current_time - *fim;
    ;
}

int enviar_info_servidor(time_t inicio)
{

    struct msg_t msg;

    long int uptime = time(NULL) - inicio;

    msg.type = MSG;
    msg.orig_uid = 0;
    msg.dest_uid = 0;
    sprintf((char *)msg.text,
            "Informações do servidor -> Clientes ativos: %i, Tempo: %li", contador_cliente, uptime);
    msg.text_len = sizeof(msg.text);

    for (int i = 0; i < 10; i++)
    {
        if (objeto_cliente[i].id > 0)
        {
            int ret = enviar_mensagem(objeto_cliente[i].fd, msg);
            if (ret < 0)
            {
                return ret;
            }
        };
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int porta = 0;
    struct msg_t msg;

    time_t inicio, fim;
    inicio = fim = time(NULL);

    for (int i = 0; i < 20; i++)
    {
        objeto_cliente[i].id = -1;
    }

    fd_set fd_set_ativo;
    fd_set fd_set_leitor;

    ret = obter_porta(argc, argv, &porta);
    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Erro ao obter porta = %i\n", errno);
        exit(EXIT_FAILURE);
    }

    int sockfd = open_socket();
    if (sockfd < 0)
    {
        fprintf(stderr, "\n ! ERRO: Erro ao abrir socket = %i\n", errno);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Socket = %i \n", sockfd);

    ret = definir_socket_addr(sockfd, porta);
    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Erro ao definir socket = %i\n", errno);
        exit(EXIT_FAILURE);
    }

    ret = escutar_socket(sockfd);
    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Erro ao escutar = %i\n", errno);
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&fd_set_ativo);
    FD_SET(sockfd, &fd_set_ativo);

    while (1)
    {
        fd_set_leitor = fd_set_ativo;

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(FD_SETSIZE, &fd_set_leitor, NULL, NULL, &timeout) < 0)
        {
            fprintf(stderr, "\n ! ERRO: Erro no select = %i\n", errno);
            exit(EXIT_FAILURE);
        }

        if (verificar_tempo(&fim) >= TIME_INFO)
        {
            if (enviar_info_servidor(inicio) < 0)
            {
                fprintf(stderr, "\n ! ERRO: Erro ao enviar informações do servidor = %i\n", errno);
                exit(EXIT_FAILURE);
            };
            fim = time(NULL);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &fd_set_leitor))
            {
                if (i == sockfd)
                {
                    int newsock = aceitar_conexao(sockfd);
                    if (newsock < 0)
                    {
                        fprintf(stderr, "\n ! ERRO: Erro de conexão = %i\n", errno);
                    }
                    else
                    {
                        fprintf(stdout, "Conexão aceita - Novo socket = %i.\n", newsock);
                        FD_SET(newsock, &fd_set_ativo);
                    }
                }
                else
                {
                    if (receber_mensagem(i, &msg) <= 0)
                    {
                        goto jp_tchau;
                    }

                    switch (msg.type)
                    {
                    case OI:
                        if (servico_oi(msg, i) < 0)
                        {
                            goto jp_tchau;
                        }
                        break;
                    case MSG:
                        if (servico_tchau(msg.dest_uid) == -1)
                        {
                            goto jp_tchau;
                        }
                        break;
                    case TCHAU:
                    jp_tchau:
                        servico_tchau(i);
                        fechar_socket(i);
                        FD_CLR(i, &fd_set_ativo);
                        fprintf(stdout, "Conexão fechada - Socket : %i.\n", i);
                        break;
                    default:
                        fprintf(stderr, "\n ! ERRO: Tipo inválido.\n");
                    }
                }
            }
        }
    }

    ret = fechar_socket(sockfd);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}