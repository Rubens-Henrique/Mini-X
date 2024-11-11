#include <stdio.h>
#include <sys/socket.h>
#include "utils.h"  // Incluindo o cabeçalho correto para funções de rede

int main(int argc, char *argv[])
{
    int ret = 0;
    int porta = 0;
    int id = 0;

    // Processar os argumentos
    ret = get_args(argc, argv, &porta, &id);
    if (ret < 0) {
        return -1;
    }

    // Abrir o socket
    int sockfd = open_socket();
    if (sockfd < 0) {
        return -1;
    }

    fprintf(stdout, "Socket = %i \n", sockfd);

    // Conectar ao servidor
    ret = connect_server(sockfd, porta);
    if (ret < 0) {
        return -1;
    }

    // Realizar o "aperto de mão"
    ret = handshake(sockfd, id);
    if (ret < 0) {
        fprintf(stderr, "\n ! ERRO: Falha Conexão\n");
        return -1;
    }
    else {
        fprintf(stdout, "===> Conexão bem sucedida . \n");
    }

    while (1) {
        struct msg_t msg;
        msg.orig_uid = id;

        // Receber mensagem
        if (receber_mensagem(sockfd, &msg) < 0) {
            fprintf(stderr, "\n ! ERRO: Erro ao receber mensagem.\n");
            break;
        }

        fprintf(stdout, "==============\n");
        if (msg.dest_uid == 0) {
            fprintf(stdout, "Mensagem Pública.\n");
        }
        else {
            fprintf(stdout, "Mensagem Privada.\n");
        }

        fprintf(stdout, "Usuário: %i\n", msg.orig_uid);
        fprintf(stdout, "Mensagem: %s \n", msg.text);
    }

    // Enviar mensagem de despedida
    struct msg_t msg;
    msg.type = TCHAU;
    msg.orig_uid = id;
    msg.dest_uid = 0;

    enviar_mensagem(sockfd, msg);

    // Fechar o socket
    ret = fechar_socket(sockfd);
    if (ret < 0) {
        return -1;
    }

    return 0;
}
