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


int crear_socket(){
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        cerrar_con_error("Error - No se pudo crear el socket", 1);
    return sock;
}

void asignar_direccion_socket(int socket, uint16_t puerto){
    struct sockaddr_in dir_propia;

    memset(&dir_propia, 0, sizeof(dir_propia));
    dir_propia.sin_family = AF_INET;
    // Para aceptar conexiones a través de cualquier interfaz, indicamos INADDR_ANY
    // htonl porque la macro INADDR_ANY está en orden de host
    dir_propia.sin_addr.s_addr = htonl(INADDR_ANY);
    dir_propia.sin_port = htons(puerto);

    // MEMSET???
    if (bind(socket, (struct sockaddr *) &dir_propia,
            (socklen_t) sizeof(dir_propia)) < 0)
        cerrar_con_error("Error - no se pudieron asignar la dirección y el "
                "puerto al socket", 1);
}

int enviar(int socket, char * mensaje, struct sockaddr_in * dir_remota){
    ssize_t nbytes;
    socklen_t tamanho = sizeof(struct sockaddr_in);

    if ((nbytes = sendto(socket, (void *) mensaje, (size_t) sizeof(mensaje),
            0, )))
}

int recibir(int socket, char * mensaje, struct sockaddr_in * dir_remota){
    ssize_t nbytes;
    socklen_t tamanho = sizeof(struct sockaddr_in);

    if ((nbytes = recvfrom(socket, (void *) mensaje, (size_t) sizeof(mensaje),
            0, (struct sockaddr *) dir_remota, &tamanho)) < 0){
        cerrar_con_error("Error en la recepción de datos", 1);
    } else if (nbytes == 0)
        fprintf(stderr, "El socket de conexión se ha cerrado\n");

    return nbytes;
}

void cerrar_socket(int socket){
    if (close(socket) < 0)
        cerrar_con_error("Error - no se pudo cerrar el socket", 1);
}
