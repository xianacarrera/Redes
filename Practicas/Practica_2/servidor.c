#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "lib.h"

/*
 * TODO:
 * perror
 * LÍMITE DEL PUERTO
 * Se puede usar inet_addr????
 * Podemos meter las funciones en una librería?
 */

int main(int argc, char * argv[]){
    uint16_t puerto;
    int sockserv;           // Socket de servidor
    int sockcon;            // Socket de conexión
    struct sockaddr_in ipportcli;
    socklen_t length_ptr;
    char mensaje[] = "Hey! Listen!\n";
    char mensaje2[] = "¡Hola desde el otro lado!\n";
    char ipcli[INET_ADDRSTRLEN];
    int nbytes;

    if (argc != 2)
        cerrar_con_error("Indicar un argumento: el puerto del servidor", 0);

    if (!comprobar_port(argv[1], &puerto))
        exit(EXIT_FAILURE);

    /********************** CREACIÓN DEL SOCKET ***************************/
    sockserv = crear_socket();
    /******************* ASIGNACIÓN DE DIRECCIÓN ***************************/

    asignar_direccion_puerto(sockserv, puerto);

    /************************** ESCUCHA *******************************/

    // Ponemos el servidor a la escucha
    // Permitimos que pueda tener hasta 5 clientes a la cola
    marcar_pasivo(sockserv, 5);

    while(1){

        /********************* ATENCIÓN A LAS CONEXIONES ********************/
        /*length_ptr = sizeof(struct sockaddr_in);
        if ((sockcon = accept(
                    sockserv, (struct sockaddr *) &ipportcli, &length_ptr
                )) < 0){
            perror("Error - no se pudo aceptar la conexión con accept()");
            exit(EXIT_FAILURE);
        }*/
        sockcon = atender(sockserv, &ipportcli, &length_ptr);

        // length_ptr como 4º argumento???
        if (inet_ntop(
                    AF_INET, (const void *) &ipportcli.sin_addr.s_addr,
                    &ipcli[0], (socklen_t) length_ptr
                ) == NULL)
            cerrar_con_error("Error al convertir la IP binaria del cliente "
                    "a texto", 1);

        printf("Conexión con la IP %s y el puerto %d\n",
                ipcli, ntohs(ipportcli.sin_port));

        // El puerto es distinto al del servidor????

        /*************************** ENVÍO DE DATOS *************************/
/* Antigua implementación
        if ((nbytes = send(
                    sockcon, (void *) mensaje,
                    (socklen_t) strlen(mensaje) + sizeof('\0'), 0)
                ) < 0){
            perror("Error en el envío de datos al cliente con send");
            exit(EXIT_FAILURE);
        }

        printf("Se han enviado %d bytes.\n", nbytes);
        printf("Longitud del mensaje original = %ld bytes\n",
                strlen(mensaje) + sizeof('\0'));*/

        // Con varios mensajes no mandamos el '\0' entre ellos
        /*
        if ((nbytes = send(
                    sockcon, (void *) mensaje,
                    (socklen_t) strlen(mensaje) + sizeof('\0'), 0)
                ) < 0){
            perror("Error en el envío de datos al cliente con send");
            exit(EXIT_FAILURE);
        }*/
        nbytes = enviar(sockcon, mensaje);

        printf("Se han enviado %d bytes.\n", nbytes);
        printf("Longitud del mensaje original = %ld bytes\n",
                strlen(mensaje) + sizeof('\0'));

        nbytes = enviar(sockcon, mensaje2);

        printf("Se han enviado %d bytes.\n", nbytes);
        printf("Longitud del mensaje original = %ld bytes\n",
                strlen(mensaje2) + sizeof('\0'));

        /************************** CIERRE DE LA CONEXIÓN *******************/

        cerrar_socket(sockcon);
    }
}
