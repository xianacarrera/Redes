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

// Quitar librerías innecesarias

int main(int argc, char * argv[]){
    uint16_t puerto;
    int sockserv;
    struct sockaddr_in direccion;
    char buffer[N];
    char * frase;
    int res_recv;
    //int leido = 0;
    int leido;

    if (argc != 3)
        cerrar_con_error("Introducir 2 argumentos: IP y puerto del "
                "servidor", 0);


    if (!comprobar_port(argv[2], &puerto))
        exit(EXIT_FAILURE);

    // Comprobamos la IP con inet_pton
    memset(&direccion, 0, sizeof(direccion));
    // Lo introducimos directamente en la estructura sockaddr_in
    if ((inet_pton(AF_INET, argv[1], (void *) &direccion.sin_addr)) < 0)
        cerrar_con_error("Error - la IP debe tener un formato IPv4 válido", 0);

    sockserv = crear_socket();


    solicitar_conexion(sockserv, direccion, puerto);


    //sleep(3);

    //res_recv = recibir(sockserv, &buffer[0]);
    //     printf("Mensaje de %d bytes recibido:\n", res_recv);
    /*
    while (leido < res_recv){
        frase = &buffer[leido];
        printf("%s", frase);
        leido += strlen(frase) + sizeof('\0');
    }*/

    // Puedo utilizar la función de mi librería??
    while ((res_recv = recibir_nbytes(sockserv, &buffer[0], 5)) > 0){
        leido = 0;
        printf("\nMensaje de %d bytes recibido:\n", res_recv);
        while (leido < res_recv){
            frase = &buffer[leido];
            printf("%s", frase);
            leido += strlen(frase) + sizeof('\0');
        }
    }


    cerrar_socket(sockserv);

    exit(EXIT_SUCCESS);
}
