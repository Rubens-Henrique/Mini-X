#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip.h>
#include <ctype.h>
#include <utils.h>

// Função para exibir uma mensagem de ajuda e finalizar o programa
void HELPCLIENT(const char *prog_name) {
    fprintf(stderr, "Uso: %s <porta> <id>\n", prog_name);
    fprintf(stderr, "Exemplo: %s 12345 1\n", prog_name);
    exit(1); // Encerra o programa com código de erro
}

void HELPSERVER(char *program_name)
{
    fprintf(stderr, "Uso incorreto. Uso esperado:\n");
    fprintf(stderr, "  %s <porta> <id>\n", program_name);
    fprintf(stderr, "\nOnde:\n");
    fprintf(stderr, "  <porta>  : número da porta (entre 1024 e 65536)\n");
    fprintf(stderr, "  <id>     : identificador do usuário (um número inteiro)\n");
    fprintf(stderr, "\nExemplo de uso:\n");
    fprintf(stderr, "  %s 12345 1\n", program_name);
}

int receber_mensagem(int sockfd, struct msg_t *msg)
{
    size_t length = sizeof(*msg);

    int ret = read(sockfd, msg, length);

    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Failed to receive message. errno: %i \n", errno);
        return -1;
    }

    if (ret == 0)
    {
        fprintf(stderr, "\n ! ERRO: No data read from socket: %i \n", sockfd);
        return -2;
    }

    return ret;
}

int enviar_mensagem(int dest_sock, struct msg_t msg)
{

    size_t length = sizeof(msg);

    int ret = write(dest_sock, &msg, length);
    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Failed to send %i message. errno: %i \n", msg.type, errno);
        return -1;
    }

    if (ret == 0)
    {
        fprintf(stderr, "\n ! ERRO: No data sended from socket: %i.\n", dest_sock);
        return -2;
    }

    return ret;
}

int connect_server(int sockfd, int port)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = INADDR_ANY;

    socklen_t length = sizeof(addr);

    int ret = connect(sockfd, (struct sockaddr *)&addr, length);

    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Failed to connect with the server. errno: %i \n", errno);
    }
    else
    {
        fprintf(stdout, "\n Succesfull connect \n");
    }

    return ret;
}

int get_args(int argc, char *argv[], int *port, int *id)
{
    if (argc != 3)
    {
        fprintf(stderr, "\n ! ERRO: Missing Arguments.\n");
        HELPCLIENT(argv[0]);
        return -1;
    }

    *port = atoi(argv[1]);
    if (*port < 1024 || *port > 65536)
    {
        fprintf(stderr, "\n ! ERRO: PORT is out of limit.\n");
        HELPCLIENT(argv[0]);
        return -1;
    }

    *id = atoi(argv[2]);
    if (*id < 1)
    {
        fprintf(stderr, "\n ! ERRO: ID must be greater than 0.\n");
        HELPCLIENT(argv[0]);
        return -1;
    }

    return 0;
}

int handshake(int sockfd, int id)
{
    struct msg_t msg;

    msg.type = OI;
    msg.orig_uid = id;
    msg.dest_uid = 0;
    msg.text_len = 0;
    msg.text[0] = '\0';

    int ret = enviar_mensagem(sockfd, msg);
    if (ret <= 0)
    {
        fprintf(stderr, "\n ! ERRO: [HANDSHAKE] Failed to send mensage.\n");
        return -1;
    }

    struct msg_t rec_msg;

    ret = receber_mensagem(sockfd, &rec_msg);
    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: [HANDSHAKE] Failed to receive mensage.\n");
        return -1;
    }

    if (rec_msg.orig_uid == msg.dest_uid)
    {
        return 0;
    }

    return -1;
}

int open_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "\n ! ERRO: Failed to create socket. errno: %i \n", errno);
    }

    return sockfd;
}

int fechar_socket(int sockfd)
{
    int ret = close(sockfd);

    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Failed to close socket. errno: %i \n", errno);
    }

    return ret;
}