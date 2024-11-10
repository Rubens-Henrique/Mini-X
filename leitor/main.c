#include <stdio.h>
#include <sys/socket.h>
#include <utils.h>

int main(int argc, char *argv[])
{
    int ret = 0;

    int porta = 0;
    int id = 0;

    ret = get_args(argc, argv, &porta, &id);
    if (ret < 0)
    {
        return -1;
    }

    int sockfd = abrir_socket();
    if (sockfd < 0)
    {
        return -1;
    }

    fprintf(stdout, "Socket = %i \n", sockfd);

    ret = conecta_servidor(sockfd, porta);
    if (ret < 0)
    {
        return -1;
    }

    ret = aperto_de_mao(sockfd, id);
    if (ret < 0)
    {
        fprintf(stderr, "\n ! ERRO: Falha no aperto de mão\n");
        return -1;
    }
    else
    {
        fprintf(stdout, "===> Aperto de mão sucedido. \n");
    }

    while (1)
    {
        struct msg_t msg;
        msg.orig_uid = id;

        if (receber_mensagem(sockfd, &msg) < 0)
        {
            fprintf(stderr, "\n ! ERRO: Erro ao receber mensagem.\n");
            break;
        }

        fprintf(stdout, "==============\n");
        if (msg.dest_uid == 0)
        {
            fprintf(stdout, "Mensagem Pública.\n");
        }
        else
        {
            fprintf(stdout, "Mensagem Privada.\n");
        }

        fprintf(stdout, "Usuário: %i\n", msg.orig_uid);
        fprintf(stdout, "Mensagem: %s \n", msg.text);
    }

    struct msg_t msg;
    msg.type = TCHAU;
    msg.orig_uid = id;
    msg.dest_uid = 0;

    enviar_mensagem(sockfd, msg);

    ret = fechar_socket(sockfd);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}