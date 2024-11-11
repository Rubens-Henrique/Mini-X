#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <utils.h>

//Coleta os dados para enviá-lo
int menu(struct msg_t *msg)
{
    char entrada[4],ch[4], text[140];

    fprintf(stdout, "\n Envia uma mensagem (1)\n");
    fprintf(stdout, "Saída (0)\n");

    if (fgets(ch, 4, stdin) == NULL)
    {
        return -1;
    }
    if (atoi(ch) == 0)
    {
        return -1;
    }
//Identificadores do destino , se não reconhecer retorna -1 e encerra conexão 
    fprintf(stdout, "Digite o ID de destino (0 - 999): ");

    if (fgets(entrada, 4, stdin) == NULL)
    {
        return -1;
    }
    fprintf(stdout, "\n");

    int dest = atoi(entrada);

   if (dest < 0 || dest > 999) {
        return -1;
    }
    fprintf(stdout, " Digite a mensagem :(máx 140 caracteres): ");
    text[0] = (char)getchar();
    if (fgets(text + 1, 139, stdin) == NULL)
    {
        return -1;
    }
    int length = strlen(text);

    if (length > 140)
    {return -1;
    }
    msg->type = MSG;
    msg->dest_uid = dest;
    msg->text_len = length;
    strcpy((char *)msg->text, text);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0,port = 0,id=0;
   
    ret = get_args(argc, argv, &port, &id);
    if (ret < 0)
    {
        return -1;
    }
    int sockfd = open_socket();
    if (sockfd < 0)
    {
        return -1;
    }

    fprintf(stdout, "Socket = %i \n", sockfd);
    if (connect_server(sockfd, port)<0)
    {
        return -1;
    }
//Confere  se a conexão foi feita 
    if (handshake(sockfd, id)<0) {
        fprintf(stderr, "\n ### ERRO: Falha NA comunicação \n");
        return -1;
    } else {
        fprintf(stdout, "==> Comunicação bem-sucedida \n");
    }

    while (1)
    {
         struct msg_t msg = { .orig_uid = id };
        if (menu(&msg)<0)
        {
            break;
        }
        if (enviar_mensagem(sockfd, msg) < 0){
            fprintf(stderr, "\n ### ERRO: Falha no envio da mensagem\n");
        }
    }
    // Estrutura TCHAU(encerrar conexão)
      struct msg_t msg = { .type = TCHAU, .orig_uid = id, .dest_uid = 0 };
 
     enviar_mensagem(sockfd, msg);
    if (fechar_socket(sockfd)<0)
    {
        return -1;
    }
    return 0;
}
