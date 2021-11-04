#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "lib.h"

#define N 1000


int main(int argc, char * argv[]){
    uint16_t puerto;
    int sockserv;
    struct sockaddr_in direccion;
    char buffer[N];
    char * frase;
    int res_recv;
    int leido = 0;

    if (argc != 3){
        fprintf(stderr, "Introducir 2 argumentos: IP y puerto del servidor\n");
        exit(EXIT_FAILURE);
    }


    if (!comprobar_port(argv[2], &puerto)) exit(EXIT_FAILURE);

    // Comprobamos la IP con inet_pton
    memset(&direccion, 0, sizeof(direccion));
    // Lo introducimos directamente en la estructura sockaddr_in
    if ((inet_pton(AF_INET, argv[1], (void *) &direccion.sin_addr)) < 0){
        perror("Error - la IP debe tener un formato IPv4 válido");
        exit(EXIT_FAILURE);
    }


    if ((sockserv = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error en la creación del socket");
        exit(EXIT_FAILURE);
    }


    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(puerto);


    if (connect(sockserv, (struct sockaddr *) &direccion,
            sizeof(struct sockaddr_in)) < 0){
        perror("Error en el establecimiento de la conexión (connect)");
        exit(EXIT_FAILURE);
    }


    sleep(3);

    // Como no sabemos cóom será el mensaje a recibir, indicamos como tamaño
    // máximo el tamaño del array, N
    if ((res_recv = recv(sockserv, &buffer[0], N, 0)) < 0){
        perror("Error en recv");
        exit(EXIT_FAILURE);
    } else if (res_recv == 0){
        fprintf(stderr, "El socket de conexión se ha cerrado\n");
    }

    printf("Mensaje de %d bytes recibido:\n", res_recv);

    while (leido < res_recv){
        frase = &buffer[leido];
        printf("%s", frase);
        leido += strlen(frase) + sizeof('\0');
    }


    if (close(sockserv) == -1){
        perror("Error al cerrar el socket");
        exit(EXIT_FAILURE);
    }


    exit(EXIT_SUCCESS);
}
