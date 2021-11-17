#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "lib.h"

// Quitar librerías innecesarias

#define BYTES_A_RECIBIR 5

/*
 * La gestión de errores de las funciones socket(), connect(), recv() y
 * close() se realiza en crear_socket(), solicitar_conexion(), recibir()
 * o recibir_nbytes(), y cerrar_socket(), respectivamente.
 */

int main(int argc, char * argv[]){
    int sockcli;       // Identificador del socket que se usará en la conexión
    uint16_t puerto;                 // Puerto que utiliza el servidor
    struct sockaddr_in ipportserv;   // Dirección del servidor (IP y puerto)
    char * frase;         // Variable auxiliar para la lectura de mensajes
    ssize_t res_recv;     // Número de bytes recibidos del servidor
    int leido;            // Contador de bytes imprimidos del mensaje recibido
    char buffer[N + 1];   // Cadena que guardará el mensaje recibido.
        /*
         * Como en teoría no podríamos conocer los mensajes que enviará el
         * servidor, definimos el tamaño máximo del buffer como una macro
         * accesible tanto para servidor como para el cliente a través de
         * lib.h. No obstante, tomamos un carácter más  que el valor de la
         * macro para añadir '\0' en caso de que el servidor no lo envíe.
         */


    /************************ COMPROBACIÓN DEL INPUT ***********************/

    /*
     * Si el número de bytes que queremos recibir en el apartado 1.d) es mayor
     * que el tamaño de los mensajes, podría haber un comportamiento
     * inesperado. Por precaución, paramos la ejecución.
     * No es necesario comparar con N * sizeof(char) en lugar de N, ya que
     * sizeof(char) es siempre 1.
     */
    if (BYTES_A_RECIBIR > N)
        // Imprimimos el error sin llamar a perror() (0 como segundo argumento)
        // y cerramos el programa
        cerrar_con_error("Se está intentando especificar un tamaño de "
                "recepción mayor que el tamaño máximo de un mensaje. "
                "Abortando...", 0);

    if (argc != 3)
        cerrar_con_error("Introducir 2 argumentos: IP y puerto del "
                "servidor", 0);

    /*
     * Comprobamos que el puerto del servidor sea correcto (entero dentro
     * del rango [IPPORT_USERRESERVED, 65535]).
     * De ser así, guardamos el resultado en puerto. En caso contrario, se
     * cierra el programa.
     */
    comprobar_port(argv[2], &puerto);

    // Comprobamos la IP con inet_pton, aprovechando para guardarla
    // directamente en la estructura sockaddr_in
    // Limpiamos el área de memoria de la estructura
    memset(&ipportserv, 0, sizeof(ipportserv));
    /*
     * A inet_pton le pasamos:
     *  - AF_INET, pues solo trabajamos con direcciones IPv4.
     *  - La cadena de texto que vamos a pasar a binario.
     *  - La dirección de memoria donde almacenaremos la IP resultante, que se
     *          guardará como una struct in_addr.
     * Si la función no da error, significará que la IP es válida.
     */
    if ((inet_pton(
                AF_INET, (const char *) argv[1], (void *) &ipportserv.sin_addr
            )) < 0)
        cerrar_con_error("Error - la IP debe tener un formato IPv4 válido", 0);
    // Cubrimos el resto de campos de la dirección
    ipportserv.sin_family = AF_INET;       // Usamos IPv4
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


    /************************ RECEPCIÓN DEL MENSAJE ************************/
    // Apartado 1.c) Esperamos 3 s para que al servidor le de tiempo a enviar
    // ambos mensajes
    // sleep(3);

    /*
     * Por si el mensaje que envía el servidor no acaba en '\0', llenamos
     * el buffer con '\0', de forma que aseguramos que siempre se encuentre
     * el final de la cadena. Además, al tener tamaño N + 1, habrá '\0'
     * incluso si se reciben N bytes, el máximo especificado.
     */
    memset(&buffer[0], '\0', sizeof(buffer));

    /*
     * Llamamos a recv(), que espera a que lleguen datos mientras sockcli,
     * el socket de la conexión, continúa abierto (ya que indicamos el
     * comportamiento por defecto).
     * El número de bytes recibidos se guarda en res_recv.
     */
//    res_recv = recibir(sockcli, &buffer[0]);

//    printf("¡Ha llegado un mensaje del servidor!\n");
    // Aunque el formato estándar para ssize_t es %zd, los sistemas
    // antiguos no lo soportan. Por seguridad, realizamos un cast a long
//    printf("\tNúmero de bytes recibidos: %ld\n", (long) res_recv);

    /*
     * El mensaje que envíe el servidor puede contener caracteres nulos
     * intermedios. Para asegurar que lo mostramos en su totalidad,
     * llevamos la cuenta del número de bytes leídos, teniendo en cuenta
     * que sizeof(char) = 1.
     */
//    printf("\tMensaje: ");
//    leido = 0;
//    while (leido < res_recv){
//        frase = &buffer[leido]; // Nos colocamos en el primer carácter sin leer
//        printf("%s", frase);    // Imprimimos hasta encontrar '\0'
        /*
         * Aumentamos leido para que apunte al carácter siguiente a '\0'
         * Nótese sizeof('\0') = 4, ya que '\0' es un carácter literal, que en
         * C se interpreta como un int.
         * Aunque la cadena recibida no termine en '\0', lo hemos añadido
         * artificialmente, de forma que estas operaciones tienen sentido.
         */
//        leido += strlen(frase) + sizeof('\0');
//    }
//    printf("\n");

    /*
     * Llamamos a recibir_nbytes(), una modificación de recibir() que
     * especifica en recv() un cierto número de bytes a recoger del envío
     * (el tercer argumento de recibir_nbytes()).
     *
     * Con este bucle, mantendremos la conexión abierta hasta que se reciban 0
     * bytes o el servidor cierre su socket de conexión. De esta forma,
     * aseguramos la recepción de todos los mensajes que envíe el servidor.
     *
     * En cada iteración vamos a sobreescribir el buffer. Ahora bien, solo
     * aseguraríamos que la lectura de la cadena es correcta si el servidor
     * siempre envía '\0' al final de sus mensajes, pues podría haber datos
     * de anteriores envíos guardados en el buffer. Por precaución, vaciamos
     * lo usado en la cadena antes de cada recepción.
     */
    while ((res_recv = recibir_nbytes(sockcli, &buffer[0],
            (size_t) BYTES_A_RECIBIR)) > 0){
        leido = 0;      // Reiniciamos leido

        printf("\n¡Ha llegado un mensaje del servidor!\n");
        // Aunque el formato estándar para ssize_t es %zd, los sistemas
        // antiguos no lo soportan. Por seguridad, realizamos un cast a long
        printf("\tNúmero de bytes recibidos: %ld\n", (long) res_recv);

        /*
         * El mensaje que envíe el servidor puede contener caracteres nulos
         * intermedios. Para asegurar que lo mostramos en su totalidad,
         * llevamos la cuenta del número de bytes leídos, teniendo en cuenta
         * que sizeof(char) = 1.
         */
        printf("\tMensaje: ");
        while (leido < res_recv){
            frase = &buffer[leido];   // Primer carácter sin leer
            printf("%s\n", frase);    // Imprimimos hasta encontrar '\0'
            /*
             * Aumentamos leido para que apunte al carácter siguiente a '\0'.
             * Nótese que strlen() no cuenta '\0', de modo que tenemos que
             * debemos sumar 1 a la longitud de la cadena.
             * Aunque el mensaje recibido no termine en '\0', lo hemos añadido
             * artificialmente, de forma que estas operaciones tienen sentido.
             */
            leido += strlen(frase) + 1;
        }

        /*
         * Vaciamos la memoria que podríamos haber usado. En este caso, dado
         * que trabajamos con arrays de caracteres, número de bytes y número
         * de posiciones son comparables, pues sizeof(char) es 1. Así,
         * llenaremos BYTES_A_RECIBIR posiciones con '\0'.
         */
        memset(&buffer[0], '\0', BYTES_A_RECIBIR);
    }

    /************************** CIERRE DE LA CONEXIÓN *******************/
    // Cerramos el socket a través del cual se estableció la conexión.
    cerrar_socket(sockcli);

    // En este programa no se ha reservado memoria

    printf("Cerrando cliente...\n\n");

    exit(EXIT_SUCCESS);
}
