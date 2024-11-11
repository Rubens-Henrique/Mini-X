#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/ip.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> // Inclui a função close
#include <utils.h>
#include <sys/select.h>

#define MAX_CLIENTES 20
#define MAX_LEITORES 10

int contador_cliente = 0;

struct {
    int id;
    int fd;
} objeto_cliente[MAX_CLIENTES];

// Função para aceitar uma nova conexão
int aceitar_conexao(int sockfd) {
    struct sockaddr addr;
    socklen_t length = sizeof(addr);
    int ret = accept(sockfd, &addr, &length);
    if (ret < 0) {
        perror("Erro ao aceitar conexão");
    }
    return ret;
}

// Função para definir o endereço do socket
int definir_socket_addr(int sockfd, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(sockfd, (struct sockaddr *)(&addr), sizeof(struct sockaddr));
    if (ret < 0) {
        perror("Erro ao vincular endereço");
    }
    return ret;
}

// Função para configurar o socket para escuta
int escutar_socket(int sockfd) {
    int ret = listen(sockfd, FILA_MAXIMA);
    if (ret < 0) {
        perror("Erro ao configurar socket para escuta");
    }
    return ret;
}

// Função para obter a porta do servidor a partir dos argumentos
int obter_porta(int argc, char *argv[], int *port) {
    if (argc != 2) {
        fprintf(stderr, "Erro: Faltando argumentos.\n");
        HELPSERVER(argv[0]);
        return -1;
    }
    *port = atoi(argv[1]);
    if (*port < 1024 || *port > 65536) {
        fprintf(stderr, "Erro: Porta fora dos limites.\n");
        HELPSERVER(argv[0]);
        return -1;
    }
    return 0;
}

// Função para processar a mensagem de inicialização do cliente
int servico_oi(struct msg_t msg, int sockfd) {
    int id = msg.orig_uid;
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (objeto_cliente[i].id == id || objeto_cliente[i].id == id + 1000) {
            fprintf(stderr, "Erro: ID já em uso.\n");
            enviar_mensagem(sockfd, msg);
            return -1;
        }
    }

    msg.orig_uid = msg.dest_uid;
    msg.dest_uid = id;

    int limite_superior = (id < 1000) ? MAX_LEITORES : MAX_CLIENTES;
    int inicio = (id > 1000) ? MAX_LEITORES : 0;

    for (int i = inicio; i < limite_superior; i++) {
        if (objeto_cliente[i].id == -1) {
            objeto_cliente[i].id = id;
            objeto_cliente[i].fd = sockfd;
            contador_cliente++;
            return enviar_mensagem(sockfd, msg);
        }
    }

    fprintf(stderr, "Erro: Limite de clientes atingido.\n");
    return -1;
}

// Função para finalizar a conexão de um cliente
int servico_tchau(int sockfd) {
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (objeto_cliente[i].fd == sockfd) {
            objeto_cliente[i].id = -1;
            objeto_cliente[i].fd = -1;
            contador_cliente--;
            break;
        }
    }
    struct msg_t tchau_msg = {TCHAU, 0, -1, 0, ""};
    return enviar_mensagem(sockfd, tchau_msg);
}

// Função para enviar informações do servidor
int enviar_info_servidor(time_t inicio) {
    struct msg_t msg;
    long int uptime = time(NULL) - inicio;

    // Converte msg.text para char* usando cast, para compatibilidade com snprintf
    snprintf((char *)msg.text, sizeof(msg.text), "Informações -> Clientes ativos: %d, Tempo: %ld", contador_cliente, uptime);
    msg.type = MSG;
    msg.orig_uid = 0;
    msg.dest_uid = 0;
    msg.text_len = strlen((char *)msg.text);  // Converte msg.text para char* usando cast

    for (int i = 0; i < MAX_LEITORES; i++) {
        if (objeto_cliente[i].id > 0) {
            int ret = enviar_mensagem(objeto_cliente[i].fd, msg);
            if (ret < 0) {
                return ret;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int porta;
    if (obter_porta(argc, argv, &porta) < 0) exit(EXIT_FAILURE);

    int sockfd = open_socket();
    if (sockfd < 0) exit(EXIT_FAILURE);

    if (definir_socket_addr(sockfd, porta) < 0) exit(EXIT_FAILURE);
    if (escutar_socket(sockfd) < 0) exit(EXIT_FAILURE);

    fprintf(stdout, "Socket configurado: %i\n", sockfd);

    fd_set fd_set_ativo, fd_set_leitor;
    FD_ZERO(&fd_set_ativo);
    FD_SET(sockfd, &fd_set_ativo);

    time_t inicio = time(NULL), fim = inicio;

    for (int i = 0; i < MAX_CLIENTES; i++) {
        objeto_cliente[i].id = -1;
    }

    while (1) {
        fd_set_leitor = fd_set_ativo;
        struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

        if (select(FD_SETSIZE, &fd_set_leitor, NULL, NULL, &timeout) < 0) {
            perror("Erro no select");
            exit(EXIT_FAILURE);
        }

        if (time(NULL) - fim >= TIME_INFO) {
            enviar_info_servidor(inicio);
            fim = time(NULL);
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &fd_set_leitor)) {
                if (i == sockfd) {
                    int newsock = aceitar_conexao(sockfd);
                    if (newsock < 0) {
                        fprintf(stderr, "Erro na conexão.\n");
                    } else {
                        fprintf(stdout, "Conexão aceita: socket %i\n", newsock);
                        FD_SET(newsock, &fd_set_ativo);
                    }
                } else {
                    struct msg_t msg;
                    if (receber_mensagem(i, &msg) <= 0) {
                        goto tchau_cliente;
                    }

                    switch (msg.type) {
                        case OI:
                            if (servico_oi(msg, i) < 0) goto tchau_cliente;
                            break;
                        case MSG:
                            if (enviar_mensagem(objeto_cliente[msg.dest_uid].fd, msg) < 0) goto tchau_cliente;
                            break;
                        case TCHAU:
                        tchau_cliente:
                            servico_tchau(i);
                            close(i);
                            FD_CLR(i, &fd_set_ativo);
                            break;
                        default:
                            fprintf(stderr, "Erro: Tipo de mensagem desconhecido.\n");
                            goto tchau_cliente;
                    }
                }
            }
        }
    }
    return 0;
}
