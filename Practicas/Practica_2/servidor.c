#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h>
#include "lib.h"


/*
 * La gestión de errores de socket(), bind(), listen(), accept(), send()
 * y close() se realiza en las funciones crear_socket(),
 * asignar_direccion_puerto(), marcar_pasivo(), atender(), enviar() y
 * cerrar_socket(), respectivamente.
 */

#define MAX_CLIENTES 10  // Número total de clientes que atenderá el servidor
#define MAX_COLA 3       // Número máximo de clientes esperando ser atendidos


int main(int argc, char * argv[]){
    int sockserv;                    // Socket del servidor
    int sockcon;                     // Socket de la conexión con el cliente
    char mensaje1[] = "Hey! Listen!";    // Primer mensaje que se enviará
    char mensaje2[] = "¡Hola desde el otro lado!";    // Segundo mensaje
    uint16_t puerto;          // Puerto introducido pasado a formato entero
    struct sockaddr_in ipportcli;    // Dirección del cliente (IP y puerto)
    ssize_t nbytes;                  // Número de bytes enviados al cliente
    int nclientes = 0;               // Número de clientes que se han atendido
    char ipcli[INET_ADDRSTRLEN];     // Cadena para almacenar la IP del
                                     // cliente en formato de texto


    /************************ COMPROBACIÓN DEL INPUT ***********************/

    // El usuario debe introducir el puerto desde el que el servidor
    // escuchará las peticiones de los clientes
    if (argc != 2)
        // Imprimimos un error sin llamar a perror() (0 como segundo
        // argumento y cerramos el programa
        cerrar_con_error("Indicar un argumento: el puerto del servidor", 0);

    /*
     * Si el puerto no es válido, comprobar_port() finaliza la ejecución.
     * Se comrpueba también que el puerto sea mayor que IPPORT_USERRESERVED.
     * Si es válido, lo convierte a un uint16_t y guarda el resultado en puerto
     */
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
     * Permitimos hasta MAX_COLA clientes esperando ser atendidos. De esta
     * forma, se podrá atender múltiples conexiones secuenciales seguidas.
     */
    marcar_pasivo(sockserv, MAX_COLA);


    /*********************** ATENCIÓN A LAS CONEXIONES *********************/

    // Para poder controlar el cierre del socket del servidor, atendemos
    // un número de clientes fijo.
    while(nclientes++ < MAX_CLIENTES){
         // Aceptamos la conexión de un cliente usando accept()
         // Guardamos su dirección (puerto + IP) en ipportcli
        sockcon = atender(sockserv, &ipportcli);

        /*
         * Convertimos la IP binaria del cliente a formato textual. Indicamos:
         *      - AF_INET, pues trabajamos solo con direcciones IPv4.
         *      - La dirección a convertir.
         *      - La cadena donde se guardará el resultado.
         *      - El número de bytes disponibles en la cadena (la definimos
         *      de tamaño el máximo posible para una IPv4).
         */
        if (inet_ntop(
                    AF_INET, (const void *) &ipportcli.sin_addr.s_addr,
                    &ipcli[0], (socklen_t) INET_ADDRSTRLEN
                ) == NULL)
            // Imprimimos un error llamando a perror() (segundo argumento !0)
            // y cerramos el programa
            cerrar_con_error("Error al convertir la IP binaria del cliente "
                    "a texto", 1);

        printf("Se ha establecido una conexión con el cliente con:\n");
        printf("\tIP: %s\n", ipcli);
        printf("\tPuerto: %" PRIu16 "\n\n", ntohs(ipportcli.sin_port));
        // El puerto se pasa de orden de red a orden de host antes de
        // imprimir, para lo cual se usa el formato de uint16_t, PRIu16.


        /*************************** ENVÍO DE DATOS *************************/

        // Enviamos el primer mensaje
        // Guardamos en nbytes el número de bytes transmitidos
        nbytes = enviar(sockcon, mensaje1);

        // Aunque el formato estándar para ssize_t es %zd, los sistemas
        // antiguos no lo soportan. Por seguridad, realizamos un cast a long
        printf("***** Envío 1 *****\n");
        printf("\tSe han enviado %ld bytes.\n", (long) nbytes);
        printf("\tLongitud del mensaje original: %ld bytes\n",
                strlen(mensaje1) + 1);
        printf("\tMensaje original: %s\n\n", mensaje1);
        /*
         * Como sizeof(char) es siempre 1, strlen(mensaje) y
         * strlen(mensaje) * sizeof(char) son equivalentes. Añadimos
         * 1 byte extra por el carácter nulo, que siempre enviamos.
         */

        // Enviamos el segundo mensaje al mismo cliente
        nbytes = enviar(sockcon, mensaje2);

        printf("***** Envío 2 *****\n");
        printf("\tSe han enviado %ld bytes.\n", (long) nbytes);
        printf("\tLongitud del mensaje 2 original: %ld bytes\n",
                strlen(mensaje2) + 1);
        printf("\tMensaje 2 original: %s\n\n", mensaje2);


        /************************** CIERRE DE LA CONEXIÓN *******************/

        // Cerramos el socket de conexión (no el del servidor)
        cerrar_socket(sockcon);
    }

    /*********************** CIERRE DEL SERVIDOR ****************************/

    printf("Se ha alcanzado el número de clientes máximo: %d\n", MAX_CLIENTES);

    // Tras atender a los clientes, cerramos el socket del servidor
    cerrar_socket(sockserv);

    // En este programa no se ha reservado memoria

    printf("Cerrando servidor...\n");
    exit(EXIT_SUCCESS);
}
