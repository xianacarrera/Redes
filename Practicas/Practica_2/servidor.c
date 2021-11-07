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

#DEFINE MAX_COLA 5     // Número máximo de clientes esperando ser atendidos

int main(int argc, char * argv[]){
    int sockserv;                    // Socket del servidor
    int sockcon;                     // Socket de la conexión con el cliente
    char mensaje1[] = "Hey! Listen!\n";    // Primer mensaje que se enviará
    char mensaje2[] = "¡Hola desde el otro lado!\n";    // Segundo mensaje
    uint16_t puerto;          // Puerto introducido pasado a formato entero
    struct sockaddr_in ipportcli;    // Dirección del cliente (IP y puerto)
    char ipcli[INET_ADDRSTRLEN];     // Cadena para almacenar la IP del
                                     // cliente en formato de texto
    int nbytes;                      // Número de bytes enviados al cliente


    /************************ COMPROBACIÓN DEL INPUT ***********************/

    // El usuario debe introducir el puerto desde el que el servidor
    // escuchará las peticiones de los clientes
    if (argc != 2)
        cerrar_con_error("Indicar un argumento: el puerto del servidor", 0);

    // Si el puerto no es válido, comprobar_port() finaliza el programa
    // Si es válido, lo convierte a un uint16_t y guarda el resultado en puerto
    comprobar_port(argv[1], &puerto);


    /************************* CREACIÓN DEL SOCKET *************************/

    // Generamos un socket de dominio IPv4 y orientado a conexión (TCP)
    // sockserv guardará el entero identificador del socket
    sockserv = crear_socket();

    /************************ ASIGNACIÓN DE DIRECCIÓN **********************/

    /*
     * Asignamos una dirección IP y un puerto al socket que acabamos de crear.
     * Como IP elegimos INADDR_ANY (escuchará a través de cualquier interfaz).
     * Como puerto elegimos el indicado por el usuario.
     */
    asignar_direccion_puerto(sockserv, puerto);

    /******************************* ESCUCHA *******************************/

    /*
     * Ponemos el servidor a la escucha, esto es, lo marcamos como pasivo
     * (podrá escuchar las solicitudes de conexión de clientes)
     * Permitimos que pueda tener hasta 5 clientes esperando ser atendidos.
     */
    marcar_pasivo(sockserv, MAX_COLA);


    /*********************** ATENCIÓN A LAS CONEXIONES *********************/
    while(1){
        sockcon = atender(sockserv, &ipportcli, &length_ptr);

        printf("length_ptr: %d\n", length_ptr);
        printf("INET_ADDRSTRLEN: %d\n", INET_ADDRSTRLEN);

        // length_ptr como 4º argumento???
        if (inet_ntop(
                    AF_INET, (const void *) &ipportcli.sin_addr.s_addr,
                    &ipcli[0], (socklen_t) INET_ADDRSTRLEN
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
