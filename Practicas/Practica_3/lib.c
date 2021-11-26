#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "lib.h"

void cerrar_con_error(char * mensaje, int con_errno){
    if (con_errno)
        perror(mensaje);
    else
        fprintf(stderr, "%s\n", mensaje);
    exit(EXIT_FAILURE);
}

/*
 * Función que comprueba si una determinada cadena de caracteres tiene un
 * formato de puerto válido para el programa, esto es, que es un entero
 * en el rango [IPPORT_USERRESERVED, 65535].
 *
 * Se trata de una adaptación de la función comprobar_port qie utilicé en
 * la práctica 1.
 *
 * Si en efecto, se trata de un puerto correcto, la función guarda en
 * port_formateado el entero correspondiente.
 * En caso contrario, se imprime una descripción del problema y se corta la
 * ejecución del programa.
 *
 * Parámetros:
 * const char * port -> (entrada) cadena con el puerto a convertir
 * uint16_t * port_formateado -> (salida) dirección del uint16_t donde se
 *      guardará el puerto en formato entero. uint16_t es un entero sin signo
 *      de 16 bits, el tipo definido para puertos (cabecera inttypes.h).
 *
 */
void comprobar_port(const char * port, uint16_t * port_formateado){
    // Para comprobar si el puerto es válido, usaremos strtoul en lugar de
    // atoi, que no permitiría distinguir el puerto 0 de un error.

    char * final_conversion;  // Indica dónde ha terminado de convertir strtoul
    unsigned long port_numerico;     // Resultado de strtoul

    /*
     * A strtoul le pasamos como argumentos:
     *      1) la cadena de texto que queremos convertir a formato numérico
     *      2) un puntero, que quedará apuntando a la dirección donde finalice
     *         la conversión (la siguiente al último del primer bloque de
     *         dígitos encontrado en la cadena)
     *      3) la base del número, en este caso, decimal
     * Nos devolverá un unsigned long
     */
    port_numerico = strtoul(port, &final_conversion, 10);

    // Si la cadena es no vacía y toda ella es numérica, final_conversion debe
    // apuntar a la dirección del carácter nulo
    if (*port == '\0' || *final_conversion != '\0')
        cerrar_con_error("El puerto no es válido", 0);

    // Comprobamos que no haya tenido lugar un overflow
    // ULONG_MAX es una macro definida en limits.h
    if (port_numerico == ULONG_MAX && errno == ERANGE){
        fprintf(stderr, "El puerto está fuera del rango "
                "representable\n");
        cerrar_con_error("Error del sistema", 1); // Imprimos el error de errno
    }

    if (port_numerico > 65535){
        fprintf(stderr, "El puerto está fuera del rango "
                "válido para este programa: [%d, 65535]\n\n",
                IPPORT_USERRESERVED);
        exit(EXIT_FAILURE);
    }

    // El input del usuario es correcto
    *port_formateado = (uint16_t) port_numerico;
}

/*
 * Función que encapsula socket() y su gestión de errores.
 * Es utilizada por el servidor y el cliente.
 *
 * Se crea un nuevo socket para direcciones IPv4 y orientado a conexión (TCP).
 * El descriptor generado se devuelve como valor de retorno.
 * En caso de error, se imprime un mensaje descriptivo junto al significado
 * del valor al que se ha dejado errno y se finaliza la ejecución.
 */
int crear_socket(){
    int sock;         // Valor de retorno de socket()

    /*
     * Pasamos como argumentos:
     * - AF_INET, pues el dominio de comunicación serán direcciones IPv4
     * - SOCK_STREAM, para establecer un servicio orientado a conexión (TCP).
     * - 0, para utilizar el protocolo por defecto para la combinación
     *      AF_INET y SOCK_STREAM (IPPROTO_TCP).
     */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        // Si socket() devuelve -1, ha ocurrido un error
        cerrar_con_error("Error - No se pudo crear el socket", 1);
    return sock;      // Devolvemos el identificador del socket
}


/*
 * Función que encapsula bind() y su gestión de errores.
 * Es utilizada por el servidor.
 *
 * Se asigna una dirección (un par IP - puerto) a un socket previamente creado.
 * Como IP se establecerá INADDR_ANY, de forma que el servidor pueda escuchar
 * a través de cualquier interfaz.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 *      - sockserv -> Entrada. Identificador del socket al que se asignará la
 *                    dirección.
 *      - puerto -> Entrada. Número de puerto al que se conectará el socket.
 */
void asignar_direccion_socket(int socket, uint16_t puerto){
    struct sockaddr_in dir_propia;

    memset(&dir_propia, 0, sizeof(dir_propia));    // "Limpiamos" el espacio
    dir_propia.sin_family = AF_INET;     // Utilzaremos una dirección IPv4
    // Para aceptar conexiones a través de cualquier interfaz, indicamos
    // INADDR_ANY. Para convertirla de orden de host a red, usamos htonl()
    dir_propia.sin_addr.s_addr = htonl(INADDR_ANY);
    dir_propia.sin_port = htons(puerto);
    // Guardamos el puerto pasado como argumento, pasado de orden de host a
    // orden de red con htons()

    /*
     * Asignamos la dirección al socket, con argumentos:
     * - el entero identificador del socket.
     * - un puntero a la estructura con la dirección a asignar, utilizando
     *      el tipo genérico struct sockaddr *.
     * - el tamaño de la anterior estructura.
     */
    if (bind(socket, (struct sockaddr *) &dir_propia,
            (socklen_t) sizeof(dir_propia)) < 0)
        cerrar_con_error("Error - no se pudieron asignar la dirección y el "
                "puerto al socket", 1);
}

/*
 * Función que encapsula send() y su gestión de errores.
 * Es utilizada por el servidor y por el cliente.
 *
 * Se envía un mensaje pasado como argumento al otro extremo de la conexión
 * establecida sobre un socket. Se intenta enviar la totalidad del mensaje.
 * La función devuelve el número de bytes transmitidos, pero no se puede
 * asegurar que se hayan recibido correctamente.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 * - sockcon -> Entrada. Identificador del socket de la conexión.
 * - mensaje -> Entrda. Datos a enviar.
 *//*
ssize_t enviar(int socket, void * mensaje, struct sockaddr_in * dir_remota){
    ssize_t nbytes;  // Guardará el número de bytes transmitidos

    if ((nbytes = sendto(
                socket, mensaje, (size_t) (strlen(mensaje) + 1), 0,
                (struct sockaddr *) dir_remota,
                (socklen_t) sizeof(struct sockaddr_in))
            ) < 0)
        // strlen(mensaje) es igual a strlen(mensaje) * sizeof(char),
        // dado que sizeof(char) siempre es 1
        cerrar_con_error("Error en el envío de datos", 1);

    return nbytes;
}*/

// SE PUEDE USAR sizeof() en lugar de strlen??????????
// Pasamos el segundo argumento como void para admitir tanto char * como floats
// ya que de igual modo, en el caso de los floats tendríamos que pasar el argumento tamaño
// (al pasar una string podemos calcular su tamaño con strlen, pero no podemos usar
// sizeof con un puntero a floats porque no podemos encontrar el final, necesitamos
// pasar su tamaño)
ssize_t enviar(int socket, void * mensaje, struct sockaddr_in * dir_remota,
        size_t tam){
    ssize_t nbytes;  // Guardará el número de bytes transmitidos

    if ((nbytes = sendto(
                socket, mensaje, tam, 0, (struct sockaddr *) dir_remota,
                (socklen_t) sizeof(struct sockaddr_in))
            ) < 0)
        // strlen(mensaje) es igual a strlen(mensaje) * sizeof(char),
        // dado que sizeof(char) siempre es 1
        cerrar_con_error("Error en el envío de datos", 1);

    return nbytes;
}


/*
 * Función que encapsula recv() y su gestión de errores.
 * Es utilizada por el servidor y por el cliente.
 *
 * Se reciben datos pasados a través de la conexión establecida sobre el socket
 * de identificador sockcon. Se intenta recibir todos los datos que contiene el
 * mensaje, teniendo en cuenta que se ha impuesto un límite de longitud igual a
 * N (macro definida en lib.h). La función devuelve el número de bytes
 * recibidos.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 * - sockcon -> Entrada. Identificador del socket de la conexión.
 * - buffer -> Salida. Puntero al buffer donde se guardará el mensaje recibido.
 */
ssize_t recibir(int socket, char * buffer, struct sockaddr_in * dir_remota,
        size_t numbytes){
    ssize_t nbytes;       // Número de bytes recibidos
    socklen_t tamanho = sizeof(struct sockaddr_in);

    if (numbytes == -1) numbytes = N;     // Valor por defecto

    /*
     * Llamamos a recv() con argumentos:
     * - el entero identificador del socket de la conexión
     * - un puntero al buffer donde guardar los datos
     * - el máximo de bytes a recibir. Como no sabemos cóom será el mensaje,
     *      indicamos el tamaño del array, N
     * - las opciones predeterminadas (0)
     */
    if ((nbytes = recvfrom(socket, (void *) buffer, (size_t) numbytes, 0,
        (struct sockaddr *) dir_remota, &tamanho)) < 0){
        perror("Error en la recepción de datos");
        exit(EXIT_FAILURE);
    } else if (nbytes == 0){
        // No se ha recibido ningún byte o el socket se ha cerrado
        printf("\nNo se han recibido datos\n");
    }

    return nbytes;
}


// El receptor pone como máximo recibir 1000 floats
ssize_t recibir_floats(int socket, float * buffer,
            struct sockaddr_in * dir_remota){
    ssize_t nbytes;       // Número de bytes recibidos
    socklen_t tamanho = sizeof(struct sockaddr_in);

    /*
     * Llamamos a recv() con argumentos:
     * - el entero identificador del socket de la conexión
     * - un puntero al buffer donde guardar los datos
     * - el máximo de bytes a recibir. Como no sabemos cóom será el mensaje,
     *      indicamos el tamaño del array, N
     * - las opciones predeterminadas (0)
     */
    if ((nbytes = recvfrom(socket, (void *) buffer, (size_t) N * sizeof(float),
        0, (struct sockaddr *) dir_remota, &tamanho)) < 0){
        perror("Error en la recepción de datos");
        exit(EXIT_FAILURE);
    } else if (nbytes == 0){
        // No se ha recibido ningún byte o el socket se ha cerrado
        printf("\nNo se han recibido datos\n");
    }

    return nbytes;
}

void cerrar_socket(int socket){
    if (close(socket) < 0)
        cerrar_con_error("Error - no se pudo cerrar el socket", 1);
}
