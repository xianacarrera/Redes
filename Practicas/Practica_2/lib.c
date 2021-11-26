#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "lib.h"

/*
 * Xiana Carrera Alonso
 * Redes - Práctica
 * Curso 2021/2022
 *
 * Fichero .c de la librería lib.h, con funciones de utilidad para los
 * programas servidor y cliente utilizados en la practica 2.
 */


/*
 * Función auxiliar que cierra imprime un mensaje de error y, opcionalmente,
 * la descripción del error guardado en errno.
 *
 * Parámetros (ambos de entrada):
 *   - mensaje -> Descripción personalizada del error que será imprimida.
 *   - ver_errno -> !0 para indicar mensaje junto a la descripción de errno,
 *                 0 para mostrar solo el argumento mensaje.
 */
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
 * Si, en efecto, se trata de un puerto correcto, la función guarda en
 * port_formateado el entero correspondiente.
 * En caso contrario, se imprime una descripción del problema y se corta la
 * ejecución del programa.
 *
 * Parámetros:
 * const char * port -> Entrada. Cadena con el puerto a convertir
 * uint16_t * port_formateado -> Salida. Dirección del uint16_t donde se
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

    /*
     * Los puertos deben ser menores o iguales a 65535
     * Además, debemos tomar uno de los puertos reservados para uso explícito
     * (aquellos mayores o iguales a IPPORT_USERRESERVED)
     */
    if (port_numerico > 65535 || port_numerico < IPPORT_USERRESERVED){
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
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
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
void asignar_direccion_puerto(int sockserv, uint16_t puerto){
    struct sockaddr_in ipportserv;    // Representa un par IP - puerto

    memset(&ipportserv, 0, sizeof(ipportserv));    // "Limpiamos" el espacio
    ipportserv.sin_family = AF_INET;     // Utilzaremos una dirección IPv4
    // Para aceptar conexiones a través de cualquier interfaz, indicamos
    // INADDR_ANY. Para convertirla de orden de host a red, usamos htonl()
    ipportserv.sin_addr.s_addr = htonl(INADDR_ANY);
    ipportserv.sin_port = htons(puerto);
    // Guardamos el puerto pasado como argumento, pasado de orden de host a
    // orden de red con htons()

    /*
     * Asignamos la dirección al socket, con argumentos:
     * - el enteroidentificador del socket.
     * - un puntero a la estructura con la dirección a asignar, utilizando
     *      el tipo genérico struct sockaddr *.
     * - el tamaño de la anterior estructura.
     */
    if (bind(sockserv, (const struct sockaddr *) &ipportserv,
            (socklen_t) sizeof(ipportserv)) < 0)
        // Si ocurre un error, llamamos a perror y cerramos el programa
        cerrar_con_error("Error - no se pudieron asignar la dirección y el "
                "puerto al socket", 1);
}


/*
 * Función que encapsula listen() y su gestión de errores.
 * Es utilizada por el servidor.
 *
 * El socket pasado como argumento se marca como pasivo, de forma que pasa
 * a poder escuchar solicitudes de conexión provenientes de clientes.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 *      - sockserv -> Entrada. Identificador del socket que se pondrá a la
 *                    escucha.
 *      - max_espera -> Entrada. Número máximo de peticiones de clientes que
 *                    podrá haber esperando ser atendidos.
 */
void marcar_pasivo(int sockserv, unsigned int max_espera){
    /*
     * Llamamos a listen, con argumentos:
     * - el entero identificador del socket que será marcado como pasivo
     * - el número máximo de clientes que podrá tener a la cola
     */
    if (listen(sockserv, max_espera) < 0)
        cerrar_con_error("Error - no se pudo poner el socket a la escucha", 1);
}


/*
 * Función que encapsula accept() y su gestión de errores.
 * Es utilizada por el servidor.
 *
 * Se atiende la conexión de un cliente del que ha llegado una solicitud de
 * comunicación. Una vez aceptada, se genera un identificador para el socket
 * de la conexión, que es el valor de retorno de la función.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 *      - sockserv -> Entrada. Identificador del socket del servidor.
 *      - ipportcli -> Salida. Estructura que almacenará la dirección (IP y
 *                     puerto) del cliente que se va a atender.
 */
int atender(int sockserv, struct sockaddr_in * ipportcli){
    socklen_t length_ptr;    // Servirá tanto para indicar el tamaño de la
            // estructura addr a accept() como para que dicha función guarde el
            // tamaño de la dirección del cliente
    int sockcon;            // Identificador del socket de conexión

    // Guardamos en length_ptr el tamaño de la estructura sockaddr_in
    length_ptr = sizeof(struct sockaddr_in);

    /*
     * Llamamos a accept() con argumentos:
     * - el entero identificador del socket del servidor
     * - un puntero a la estructura sockaddr_in, casteada al tipo general
     *   struct sockaddr *, donde se almacenará la dirección del cliente
     * - un puntero al tamaño de la anterior estructura. La función guardará
     *   el espacio real utilizado para la dirección del cliente
     * La función quedará esperando a que llegue alguna solicitud. Tras
     * aceptarla, devolverá el identificador del socket de conexión.
     */
    if ((sockcon = accept(
                sockserv, (struct sockaddr *) ipportcli, &length_ptr
            )) < 0)
        cerrar_con_error("Error - no se pudo aceptar la conexión", 1);

    return sockcon;   // Devolvemos el socket de la conexión obtenido
}


/*
 * Función que encapsula connect() y su gestión de errores.
 * Es utilizada por el cliente.
 *
 * Se solicita el establecimiento de una conexión con un servidor, cuya
 * dirección se pasa como argumento.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 *      - sockcli -> Entrada. Identificador del socket del cliente a conectar.
 *      - direccion -> Entrada. Dirección (IP + puerto) del servidor. Esta
 *                     estructura debe pasarse ya configurada.
 */
void solicitar_conexion(int sockcli, struct sockaddr_in direccion){
    /*
     * Llamamos a connect() con argumentos:
     * - el entero identificador del socket a conectar
     * - un puntero a la estructura sockaddr_in, convertida al tipo genérico
     *   struct sockaddr *, que contiene la IP y el puerto del servidor.
     * - el tamaño en bytes de la anterior estructura.
     * Si el servidor no se está ejecutando, tendrá lugar un error.
     */
    if (connect(sockcli, (struct sockaddr *) &direccion,
            (socklen_t) sizeof(struct sockaddr_in)) < 0)
        cerrar_con_error("Error al solicitar la conexión con el servidor", 1);
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
 */
ssize_t enviar(int sockcon, char * mensaje){
    ssize_t nbytes;  // Guardará el número de bytes transmitidos

    /*
     * Llamamos a send() con argumentos:
     * - el identificador del socket de conexión
     * - un puntero al buffer con los datos a transmitir
     * - el número de bytes a enviar, que en este caso será todo el buffer.
     *   Mandamos también el carácter nulo final, por lo que debemos sumar 1
     *   a strlen(). No hace falta multiplicar por sizeof(char) ya que vale 1.
     * - las opciones a usar, que dejaremos como las predeterminadas (0)
     */
    if ((nbytes = send(
                sockcon, (void *) mensaje,
                (size_t) (strlen(mensaje) + 1), 0)
            ) < 0)
        // strlen(mensaje) es igual a strlen(mensaje) * sizeof(char),
        // dado que sizeof(char) siempre es 1
        cerrar_con_error("Error en el envío de datos al cliente", 1);

    return nbytes;
}

/*
 * Función que encapsula send() y su gestión de errores.
 * Es utilizada por el servidor y por el cliente.
 *
 * Se envía un mensaje pasado como argumento al otro extremo de la conexión
 * establecida sobre un socket. Se envía solo una cierta cantidad de bytes
 * del mensaje. La función devuelve el número de bytes transmitidos, pero no
 * se puede asegurar que se hayan recibido correctamente.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 * - sockcon -> Entrada. Identificador del socket de la conexión.
 * - mensaje -> Entrda. Datos a enviar.
 * - numbytes -> Entrada. Número de bytes del mensaje que se transmitirán.
 */
ssize_t enviar_nbytes(int sockcon, char * mensaje, size_t numbytes){
    ssize_t nbytes;  // Guardará el número de bytes transmitidos

    /*
     * Llamamos a send() con argumentos:
     * - el identificador del socket de conexión
     * - un puntero al buffer con los datos a transmitir
     * - el número de bytes a enviar, indicado como argumento
     * - las opciones a usar, que dejaremos como las predeterminadas (0)
     */
    if ((nbytes = send(
                sockcon, (void *) mensaje, numbytes, 0)
            ) < 0)
        cerrar_con_error("Error en el envío de datos al cliente", 1);
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
ssize_t recibir(int sockcon, char * buffer){
    ssize_t res_recv;       // Número de bytes recibidos

    /*
     * Llamamos a recv() con argumentos:
     * - el entero identificador del socket de la conexión
     * - un puntero al buffer donde guardar los datos
     * - el máximo de bytes a recibir. Como no sabemos cóom será el mensaje,
     *      indicamos el tamaño del array, N
     * - las opciones predeterminadas (0)
     */
    if ((res_recv = recv(sockcon, (void *) buffer, (size_t) N, 0)) < 0){
        perror("Error en la recepción de datos del servidor");
        exit(EXIT_FAILURE);
    } else if (res_recv == 0){
        // No se ha recibido ningún byte o el socket se ha cerrado
        printf("\nNo se han recibido datos\n");
    }

    return res_recv;
}


/*
 * Función que encapsula recv() y su gestión de errores.
 * Es utilizada por el servidor y por el cliente.
 *
 * Se reciben datos pasados a través de la conexión establecida sobre el socket
 * de identificador sockcon. Se toman solo una cierta cantidad de bytes
 * del mensaje. La función devuelve el número de bytes recibidos.
 * En caso de error, se para la ejecución y se imprime un mensaje de error.
 *
 * Parámetros:
 * - sockcon -> Entrada. Identificador del socket de la conexión.
 * - buffer -> Salida. Puntero al buffer donde se guardará el mensaje recibido.
 * - numbytes -> Entrada. Número de bytes del mensaje que se deben recoger.
 */
ssize_t recibir_nbytes(int sockcon, char * buffer, size_t numbytes){
    ssize_t res_recv;            // Número de bytes recibidos

    /*
     * Llamamos a recv() con argumentos:
     * - el entero identificador del socket de la conexión
     * - un puntero al buffer donde guardar los datos
     * - el máximo de bytes a recibir, que hemos elegido específicamente
     * - las opciones predeterminadas (0)
     */
    if ((res_recv = recv(sockcon, (void *) buffer, numbytes, 0)) < 0){
        perror("Error en la recepción de datos del servidor");
        exit(EXIT_FAILURE);
    } else if (res_recv == 0){
        // No se ha recibido ningún byte o el socket se ha cerrado
        printf("\nNo se han recibido datos\n");
    }

    return res_recv;
}


/*
 * Función que pone un límite de inactividad a un socket.
 * Es utiilizada por el servidor.
 *
 * Se establece un tiempo máximo que el socket puede permanecer sin recibir
 * algún tipo de input.
 *
 * Parámetros:
 *      - socket -> Entrada. Identificador del socket a configurar.
 *      - s -> Entrada. Tiempo límite en segundos.
 */
void poner_limite_inactividad(int socket, time_t s){
    struct timeval tlimite;
    tlimite.tv_sec = s;         // Esperamos s segundos
    tlimite.tv_usec = 0;        // Ponemos los microsegundos a 0

    /*
     * Llamamos a la función setsockopt() para dar opciones al socket
     * Pasamos como argumento:
     * - el entero identificador del socket
     * - el nivel en el que reside la opción a cambiar (SO_RCVTIMEO)
     * - una estructura timeval donde indicamos el valor en segundos y
     *   microsegundos a usar
     * - el tamaño de la anterior estructura
     */
    if (setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, (const void *) &tlimite,
            (socklen_t) sizeof(struct timeval)) < 0)
        cerrar_con_error("No se pudo establecer un tiempo límite de "
                "inactividad", 1);
}


/*
 * Función que encapsula close() y su gestión de errores.
 * Es utilizada por el servidor y por el cliente.
 *
 * Se cierra un socket que estaba siendo utilizado.
 *
 * Parámetros:
 *      - sock -> Entrada. Identificador del socket a cerrar
 */
void cerrar_socket(int sock){
    // Llamamos a close indicando qué socket queremos cerrar
    if (close(sock) < 0)
        cerrar_con_error("Error - no se pudo cerrar el socket", 1);
}
