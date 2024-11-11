#ifndef UTILS_H_
#define UTILS_H_

#define OI 0
#define TCHAU 1
#define MSG 2

#define SERVER 0
#define READER 1
#define SENDER 2

#define FILA_MAXIMA 20
#define TIME_INFO 60

struct msg_t
{
    unsigned short int type;
    unsigned short int orig_uid;
    unsigned short int dest_uid;
    unsigned short int text_len;
    unsigned char text[141];
};

extern int receber_mensagem(int, struct msg_t *);

extern int enviar_mensagem(int, struct msg_t);

extern int open_socket();

extern int fechar_socket(int);

extern int connect_server(int, int);

extern int get_args(int, char **, int *, int *);

extern int handshake(int, int);

extern void HELPSERVER(char *program_name);

#endif