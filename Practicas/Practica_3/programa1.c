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
    uint16_t puerto_propio;
    uint16_t puerto_remoto;
    ssize_t nbytes;
    char ip_remota[INET_ADDRSTRLEN];

    if (argc != 4)
        cerrar_con_error("Son nesarios 3 argumentos:\n"
                "\tPuerto propio (desde donde se enviarán datos)\n"
                "\tIP remota (IP del equipo del programa 2)\n"
                "\tPuerto remoto (puerto del programa 2)", 0);

    comprobar_port(argv[1], &puerto_propio);

    // Comprobamos la IP con inet_pton
    memset(&dir_remota, 0, sizeof(dir_remota));
    // Lo introducimos directamente en la estructura sockaddr_in
    if ((inet_pton(AF_INET, (const char *) argv[1],
            (void *) &direccion.sin_addr)) < 0)
        cerrar_con_error("Error - la IP debe tener un formato IPv4 válido", 0);

    comprobar_port(argv[3], &puerto_remoto);

    socket = crear_socket();

    asignar_direccion_socket(socket, puerto_propio);
}
