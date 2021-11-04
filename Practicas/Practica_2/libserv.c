#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "lib.h"

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

void asignar_direccion_puerto(int sockserv, uint16_t puerto){
    struct sockaddr_in ipportserv;

    memset(&ipportserv, 0, sizeof(ipportserv));
    ipportserv.sin_family = AF_INET;
    // Para aceptar conexiones a través de cualquier interfaz, indicamos INADDR_ANY
    // htonl porque la macro INADDR_ANY está en orden de host
    ipportserv.sin_addr.s_addr = htonl(INADDR_ANY);
    ipportserv.sin_port = htons(puerto);

    // MEMSET???
    if (bind(sockserv, (struct sockaddr *) &ipportserv,
            (socklen_t) sizeof(ipportserv)) < 0)
        cerrar_con_error("Error - no se pudieron asignar la dirección y el "
                "puerto al socket", 1);
}

void marcar_pasivo(int sockserv, int max_espera){
    if (listen(sockserv, max_espera) < 0)
        cerrar_con_error("Error - no se pudo poner el socket a la escucha", 1);
}

int atender(int sockserv, struct sockaddr_in * ipportcli, int * length_ptr){
    int sockcon;
    *length_ptr = sizeof(struct sockaddr_in);
    if ((sockcon = accept(
                sockserv, (struct sockaddr *) ipportcli,
                (socklen_t *) length_ptr
            )) < 0)
        cerrar_con_error("Error - no se pudo aceptar la conexión", 1);

    return sockcon;
}

int enviar(int sockcon, char * mensaje){
    int nbytes;
    if ((nbytes = send(
                sockcon, (void *) mensaje,
                (socklen_t) strlen(mensaje) + sizeof('\0'), 0)
            ) < 0)
        cerrar_con_error("Error en el envío de datos al cliente", 1);
    return nbytes;
}

void cerrar_socket(int sockcon){
    if (close(sockcon) < 0)
        cerrar_con_error("Error - no se pudo cerrar el socket", 1);
}
