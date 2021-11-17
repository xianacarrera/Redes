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
    int socket;
    struct sockaddr_in dir_remota;
    char mensaje[N];
    uint16_t puerto;
    ssize_t nbytes;
    char ip_remota[INET_ADDRSTRLEN];



    if (argc != 2)
        cerrar_con_error("Introducir un argumento: el puerto a utilizar", 0);

    comprobar_port(argv[1], &puerto);

    socket = crear_socket();

    asignar_direccion_socket(socket, puerto);

    memset(mensaje, '\0', N);   // Por si el mensaje del server no termina en '\0'

    nbytes = recibir(socket, mensaje, &dir_remota);

    if (inet_ntop(
            AF_INET, (const void *) &dir_remota.sin_addr.s_addr,
            &ip_remota[0], (socklen_t) sizeof(ip_remota)
        ) == NULL)
        cerrar_con_error("Error al convertir la IP binaria del origen de "
                "los datos a texto", 1);

    printf("Se han recibido %ld bytes\n", nbytes);
    printf("El origen de los datos tiene dirección:\n");
    printf("\tIP: %s\n", ip_remota);
    printf("\tPuerto: %d\n", dir_remota.sin_port);

    printf("%s\n", mensaje);


    cerrar_socket(socket);
}
