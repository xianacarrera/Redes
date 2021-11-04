#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "libcli.h"


/*
 * Función que comprueba si una determinada cadena de caracteres tiene un
 * formato de puerto válido para el programa, esto es, que es un entero
 * en el rango [IPPORT_USERRESERVED, 65535].
 *
 * Se trata de una adaptación de la función comprobar_port qie utilicé en
 * la práctica 1.
 *
 * Si en efecto, se trata de un puerto correcto, la función devuelve 1 y
 * guarda en port_formateado el entero correspondiente.
 * En caso contrario, se imprime una descripción del problema y se devuelve 0.
 * La ejecución no se termina aquí, sino en las funciones principales.
 *
 * Parámetros:
 * const char * port -> (entrada) cadena con el puerto a convertir
 * uint16_t * port_formateado -> (salida) dirección del uint16_t donde se
 *      guardará el puerto en formato entero. uint16_t es un entero sin signo
 *      de 16 bits, el tipo definido para puertos (cabecera inttypes.h).
 *
 * La función devuelve:
 *      0 -> En caso de error
 *      1 -> Si la conversión se realizó correctamente
 */
short comprobar_port(const char * port, uint16_t * port_formateado){
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
    if (*port == '\0' || *final_conversion != '\0'){
        fprintf(stderr, "El puerto no es válido\n\n");
        return 0;
    }

    // Comprobamos que no haya tenido lugar un overflow
    // ULONG_MAX es una macro definida en limits.h
    if (port_numerico == ULONG_MAX && errno == ERANGE){
        fprintf(stderr, "El puerto está fuera del rango "
                "representable\n");
        perror("Error del sistema: ");   // Imprimos el error que da errno
        fprintf(stderr, "\n");
        return 0;
    }

    if (port_numerico > 65535){
        fprintf(stderr, "El puerto está fuera del rango "
                "válido para este programa: [%d, 65535]\n\n",
                IPPORT_USERRESERVED);
        return 0;
    }

    // El input del usuario es correcto
    *port_formateado = (uint16_t) port_numerico;
    return 1;
}

void cerrar_con_error(char * mensaje, int con_errno){
    if (con_errno)
        perror(mensaje);
    else
        fprintf(stderr, "%s\n", mensaje);
    exit(EXIT_FAILURE);
}


int crear_socket(){
    int sockserv;
    if ((sockserv = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        cerrar_con_error("Error - No se pudo crear el socket", 1);
    return sockserv;
}

// Hace falta un puntero para direccion???
void solicitar_conexion(int sockserv, struct sockaddr_in direccion,
        uint16_t puerto){
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(puerto);

    if (connect(sockserv, (struct sockaddr *) &direccion,
            sizeof(struct sockaddr_in)) < 0)
        cerrar_con_error("Error al solicitar la conexión con el servidor", 1);
}

int recibir(int sockserv, char * buffer){
    int res_recv;

    // Como no sabemos cóom será el mensaje a recibir, indicamos como tamaño
    // máximo el tamaño del array, N
    if ((res_recv = recv(sockserv, buffer, N, 0)) < 0){
        perror("Error en la recepción de datos del servidor");
        exit(EXIT_FAILURE);
    } else if (res_recv == 0){
        fprintf(stderr, "El socket de conexión se ha cerrado\n");
    }
    return res_recv;
}

int recibir_nbytes(int sockserv, char * buffer, int numbytes){
    int res_recv;

    // Número de bytes a recibir personalizado
    if ((res_recv = recv(sockserv, buffer, numbytes, 0)) < 0){
        perror("Error en la recepción de datos del servidor");
        exit(EXIT_FAILURE);
    } else if (res_recv == 0){
        fprintf(stderr, "El socket de conexión se ha cerrado\n");
    }
    return res_recv;
}

void cerrar_socket(int sockcon){
    if (close(sockcon) < 0)
        cerrar_con_error("Error - no se pudo cerrar el socket", 1);
}
