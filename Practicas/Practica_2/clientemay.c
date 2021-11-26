#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include "lib.h"

/*
 * Xiana Carrera Alonso
 * Redes - Práctica 2
 * Curso 2021/2022
 *
 * Este programa implementa un cliente de mayúsculas. Se encarga de leer
 * líneas de un fichero de texto pasado como argumento y enviárselas a un
 * servidor que las pasa a mayúsculas y se las reenvía. Tras recibir los datos,
 * escribe las líneas resultantes en un archivo de salida.
 *
 * Notas:
 * Cada cliente espera 2 segundos antes de empezar a enviar datos al servidor
 * para facilitar las comprobaciones del ejercicio 3 con conexiones
 * secuenciales de clientes.
 *
 * La comprobación de errores se realiza a través de las funciones de la
 * librería lib.h.
 */


int main(int argc, char * argv[]){
    FILE * fichero_ent;         // Puntero al archivo de entrada
    FILE * fichero_sal;         // Puntero al archivo de salida (mayúsculas)
    char * nombre_fichero_sal;  // Cadena con el nombre del archivo de salida
    int sockcli;                // Identificador del socket para la conexión
    uint16_t puerto;            // Puerto que utiliza el servidor
    struct sockaddr_in ipportserv;    // Guardará la dirección del servidor
    char linea[N];              // Líneas leídas del fichero de entrada
    char buffer[N];             // Buffer para la recepción de mensajes
    ssize_t nbytes;             // Número de bytes enviados o recibidos
    int i;                      // Contador para el envío de datos

    /************************ TRATAMIENTO DEL INPUT ***********************/

    // Verificamos que los argumentos sean correctos y abrimos los ficheros

    if (argc != 4)
        // Imprimimos el error sin llamar a perror() (0 como segundo argumento)
        // y cerramos el programa
        cerrar_con_error("Introducir 3 argumentos: fichero de entrada, IP y "
                "puerto del servidor", 0);

    // Abrimos el fichero de entrada en modo lectura
    if ((fichero_ent = fopen(argv[1], "r")) == NULL)
        // Imprimimos el error tras llamar a perror() (segundo argumento !0)
        // y cerramos el programa
        cerrar_con_error("No se pudo abrir el fichero de entrada", 1);

    /*
     * Reservamos memoria para el nombre del fichero de salida, que es igual
     * al del fichero de entrada pasado a mayúsculas (también la extensión).
     * No sería necesario multiplicar por sizeof(char), ya que este es 1,
     * pero lo incluimos para que el código sea más explícito.
     * Añadimos una posición extra para guardar '\0'.
     */
    if ((nombre_fichero_sal =
            (char *) malloc((strlen(argv[1]) + 1) * sizeof(char))) == NULL)
        cerrar_con_error("Error reservando memoria", 0);

    for (i = 0; i < strlen(argv[1]); i++)
        // Pasamos cada uno de los caracteres del nombre del archivo a
        // mayúsculas. Si alguno no admite conversión, se deja intacto.
        *(nombre_fichero_sal + i) = toupper(*(argv[1] + i));

    // Añadimos '\0' al final.
    *(nombre_fichero_sal + i) = '\0';

    // Abrimos el fichero de salida en modo escritura (lo creamos si no existe)
    if ((fichero_sal = fopen(nombre_fichero_sal, "w")) == NULL){
        // En el error indicamos el nombre del fichero de salida
        fprintf(stderr, "Sobreescritura/creación de %s fallida",
                nombre_fichero_sal);
        cerrar_con_error("Error", 1);
    }

    // Comprobamos que el puerto introducido sea válido. Si es así, lo
    // guardamos en puerto, convertido a uint16_t. Si no, cerramos el programa.
    comprobar_port(argv[3], &puerto);


    // Comprobamos la IP del servidor con inet_pton. La introduciremos
    // directamente en la estructura ipportserv
    memset(&ipportserv, 0, sizeof(ipportserv));  // Vaciamos la memoria

    /*
     * Llamamos a inet_pton con argumentos:
     *  - AF_INET, ya que usamos direcciones IPv4.
     *  - La cadena de texto que vamos a convertir a binario.
     *  - Un puntero al lugar donde almacenaremos la IP binaria, que se
     *          guardará como una struct in_addr.
     * Si la función no da error, significará que la IP es válida.
     */
     if ((inet_pton(
                 AF_INET, (const char *) argv[2], (void *) &ipportserv.sin_addr
             )) < 0)
        cerrar_con_error("Error - la IP debe tener un formato IPv4 válido", 0);

    // Ahora que sabemos que la IP es correcta, cubrimos el resto de campos
    ipportserv.sin_family = AF_INET;       // Usaremos IPv4
    ipportserv.sin_port = htons(puerto);   // Guardamos el puerto introducido
                // tras pasarlo a de orden de host a orden de red


    /************************* CREACIÓN DEL SOCKET *************************/

    // Generamos un socket de dominio IPv4 y orientado a conexión (TCP)
    // sockcli guardará el entero identificador del socket del cliente
    sockcli = crear_socket();

    /*********************** SOLICITUD DE CONEXIÓN *************************/

    // Solicitamos la conexión con el servidor, identificado por su dirección
    // y puerto
    solicitar_conexion(sockcli, ipportserv);

    // Si no hubo errores, se habrá creado la conexión con el servidor
    printf("Se ha establecido la conexión con el servidor de:\n");
    printf("\tIP: %s\n", argv[2]);
    printf("\tPuerto: %" PRIu16 "\n\n", puerto);
    // Usamos el formato PRIu16 para imprimir el uint16_t

    /********************* ENVÍO Y RECEPCIÓN DE DATOS  *********************/

    /*
     * fgets lee a lo sumo 1 carácter menos que lo indicado como 2º argumento.
     * Definimos N en lib.h como el tamaño máximo que puede tener una línea.
     * Devuelve NULL si hay un error o ha llegado al final del fichero
     * Podemos distinguir entre esas situaciones usando ferror()
     */
    while (fgets(linea, N, fichero_ent) != NULL){
        // Nótese que fgets añade '\0' como carácter siguiente a lo almacenado

        // (apartado 3) Para comprobar más fácilmente conexiones secuenciales
        // de clientes, esperamos 2 segundos antes de enviar los datos.
        sleep(2);

        /*
         * No enviamos N bytes, sino solo el tamaño correspondiente a lo
         * leído, junto al carácter nulo final. Se usa que sizeof(char) es 1.
         * Si send() no logra enviar todos los bytes, repetimos el envío con
         * la parte del mensaje que aún no se haya transmitido.
         */
        i = 0;
        while ((nbytes = enviar_nbytes(sockcli, &linea[i],
                strlen(&linea[i]) + 1)) != strlen(&linea[i]) + 1){
            i += nbytes;
        }

        printf("***** Envío de datos *****\n");
        // Aunque el formato estándar para ssize_t es %zd, los sistemas
        // antiguos no lo soportan. Por seguridad, realizamos un cast a long
        printf("\tSe han enviado %ld bytes.\n", (long) nbytes);
        // Comparamos cuánto hemos enviado con la longitud total del mensaje
        printf("\tLongitud del mensaje original: %ld bytes\n",
                strlen(linea) + 1);
        printf("\tMensaje original: %s\n\n", linea);

        // Recibimos la línea pasada a mayúsculas
        nbytes = recibir(sockcli, &buffer[0]);
        printf("***** Recepción de datos *****\n");
        printf("\tNúmero de bytes recibidos: %ld\n", (long) nbytes);
        printf("\tMensaje recibido: %s\n\n", buffer);

        // Escribimos la línea que se ha recibido sobre el fichero de salida,
        // sin el carácter nulo final
        if (fputs((const char *) buffer, fichero_sal) == EOF){
            cerrar_con_error("Ha tenido lugar un error al escribir en "
                    "en el archivo de salida", 0);
        }
    }

    // Comprobamos si fgets ha parado por un error o por llegar al final
    if (ferror(fichero_ent))
        cerrar_con_error("Error - No se ha podido leer hasta el final del "
                "fichero", 0);


    /********************** FINALIZACIÓN DEL PROGRAMA *********************/

    // Cerramos el socket de la conexión
    cerrar_socket(sockcli);

    // Liberamos la memoria reservada
    free(nombre_fichero_sal);

    // Cerramos los ficheros
    if (fclose(fichero_ent)){
        perror("No se pudo cerrar el fichero de entrada");
        if (fclose(fichero_sal))
            perror("No se pudo cerrar el fichero de salida");
        exit(EXIT_FAILURE);
    } else if (fclose(fichero_sal)){
        perror("No se pudo cerrar el fichero de salida");
        exit(EXIT_FAILURE);
    }
    // Separamos en los casos de que falle uno o los dos, porque sus errores
    // pueden ser distintos

    printf("Cerrando cliente...\n\n");
    exit(EXIT_SUCCESS);
}
