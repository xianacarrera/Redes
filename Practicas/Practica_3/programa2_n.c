#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "lib.h"


/*
 * Xiana Carrera Alonso
 * Redes - Práctica 2
 * Curso 2021/2022
 *
 * Este programa actúa de cliente a un servidor. Tras conectarse a él,
 * recibe un mensaje, lo imprime y se cierra.
 *
 * Notas:
 * En su estado actual, el programa implementa el comportamiento
 * correspondiente al apartado 1a) de la práctica. Las líneas comentadas que
 * comienzan por 1c) o 1d) corresponden al código de dichos apartados:
 * 29, 107 y 167-198 (tapar las líneas 124 a 148 y la línea 108).
 *
 * La gestión de errores de las funciones socket(), connect(), recv() y
 * close() se realiza en crear_socket(), solicitar_conexion(), recibir()
 * o recibir_nbytes(), y cerrar_socket(), respectivamente.
 */


// #define BYTES_A_RECIBIR 5  // Bytes que recoge recv() en cada llamada


int main(int argc, char * argv[]){
    int socket;
    uint16_t puerto;
    struct sockaddr_in dir_remota;
    ssize_t nbytes;     // Número de bytes recibidos del servidor
    char mensaje[N + 1];   // Cadena que guardará el mensaje recibido.
        /*
         * Como en teoría no podríamos conocer los mensajes que enviará el
         * servidor, definimos el tamaño máximo del buffer como una macro
         * accesible tanto para servidor como para el cliente a través de
         * lib.h. No obstante, tomamos un carácter más  que el valor de la
         * macro para añadir '\0' en caso de que el servidor no lo envíe.
         */
    char ip_remota[INET_ADDRSTRLEN];
    float numeros[N];
    int i, numfloats;

    /************************ COMPROBACIÓN DEL INPUT ***********************/

    if (argc != 2)
        cerrar_con_error("Introducir un argumento: el puerto a utilizar", 0);

    /*
     * Comprobamos que el puerto del servidor sea correcto (entero dentro
     * del rango [IPPORT_USERRESERVED, 65535]).
     * De ser así, guardamos el resultado en puerto. En caso contrario, se
     * cierra el programa.
     */
    comprobar_port(argv[1], &puerto);


    /************************* CREACIÓN DEL SOCKET *************************/

    // Generamos un socket de dominio IPv4 y orientado a conexión (TCP)
    // sockcli guardará el entero identificador del socket del cliente
    socket = crear_socket();


    /******************* ASIGNACIÓN DE DIRECCIÓN ******************/

    asignar_direccion_socket(socket, puerto);


    /************************ RECEPCIÓN DEL MENSAJE ************************/

    memset(mensaje, '\0', N + 1);   // Por si el mensaje del server no termina en '\0'
/*
    nbytes = recibir(socket, mensaje, &dir_remota, -1);

    if (inet_ntop(
            AF_INET, (const void *) &dir_remota.sin_addr.s_addr,
            &ip_remota[0], (socklen_t) sizeof(ip_remota)
        ) == NULL)
        cerrar_con_error("Error al convertir la IP binaria del origen de "
                "los datos a texto", 1);


    printf("Se han recibido %ld bytes\n", (long) nbytes);
    printf("El origen de los datos tiene dirección:\n");
    printf("\tIP: %s\n", ip_remota);
    printf("\tPuerto: %" PRIu16 "\n", dir_remota.sin_port);
    printf("Mensaje: %s\n", mensaje);
*/

    /*
    * For message-based sockets, such as SOCK_RAW, SOCK_DGRAM, and SOCK_SEQPACKET, the entire message shall be read in a single operation. If a message is too long to fit in the supplied buffer, and MSG_PEEK is not set in the flags argument, the excess bytes shall be discarded.
    */
/*
    while ((nbytes = recibir(socket, mensaje, &dir_remota, 4)) > 0){
        if (inet_ntop(
                AF_INET, (const void *) &dir_remota.sin_addr.s_addr,
                &ip_remota[0], (socklen_t) sizeof(ip_remota)
            ) == NULL)
            cerrar_con_error("Error al convertir la IP binaria del origen de "
                    "los datos a texto", 1);


        printf("Se han recibido %ld bytes\n", (long) nbytes);
        printf("El origen de los datos tiene dirección:\n");
        printf("\tIP: %s\n", ip_remota);
        printf("\tPuerto: %" PRIu16 "\n", dir_remota.sin_port);
        printf("Mensaje: %s\n", mensaje);Mensaje
    }*/

    nbytes = recibir_floats(socket, numeros, &dir_remota);
    numfloats = (int) nbytes / sizeof(float);

    printf("Se han recibido %ld bytes\n", (long) nbytes);
    printf("Por tanto, hay %d floats en el mensaje\n", numfloats);
    printf("El origen de los datos tiene dirección:\n");
    printf("\tIP: %s\n", ip_remota);
    printf("\tPuerto: %" PRIu16 "\n", dir_remota.sin_port);
    printf("Los números recibidos son:\n");
    for (i = 0; i < numfloats - 1; i++){
        printf("%f - \n", numeros[i]);
    }
    printf("%f\n\n", numeros[i]);


    /************************** CIERRE DEL SOCKET *******************/

    cerrar_socket(socket);

    // En este programa no se ha reservado memoria

    printf("\nCerrando programa2...\n\n");

    exit(EXIT_SUCCESS);
}
