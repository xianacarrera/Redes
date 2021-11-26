#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include "lib.h"

/*
 * Xiana Carrera Alonso
 * Redes - Práctica 2
 * Curso 2021/2022
 *
 * Este programa implementa un servidor de mayúsculas. Tras recibir un
 * mensaje de un cliente, lo pasa a mayúsculas y le reenvía el texto
 * convertido.
 *
 * Notas:
 * El servidor se cierra automáticamente tras 30 segundos de inactividad
 * (donde se entiende inactividad por recibir algún tipo de input).
 *
 * La comprobación de errores se realiza a través de las funciones de la
 * librería lib.h.
 */

#define MAX_COLA 3       // Número máximo de clientes esperando ser atendidos
#define T_LIMITE 30      // Número de segundos que el socket puede permanecer
                         // inactivo.

int main(int argc, char * argv[]){
    int sockserv;                    // Socket del servidor
    int sockcon;                     // Socket de la conexión con el cliente
    uint16_t puerto;          // Puerto introducido pasado a formato entero
    struct sockaddr_in ipportcli;    // Dirección del cliente (IP y puerto)
    ssize_t nbytes;                  // Número de bytes recibidos/enviados
    char linea[N];                   // Línea del fichero (el tamaño máximo, N,
                                     // se define en lib.h)
    char ipcli[INET_ADDRSTRLEN];     // Cadena para almacenar la IP del
                                     // cliente en formato de texto
    int i;                           // Contador de bytes leídos y enviados


    /************************ COMPROBACIÓN DEL INPUT ***********************/

    if (argc != 2)
        // Cerramos el programa tras imprimir un mensaje de error
        // El segundo argumento es 0 para no llamar a perror()
        cerrar_con_error("Indicar un argumento: el puerto del servidor", 0);

    /*
     * Comprobamos que el puerto introducido sea válido. Si es así,
     * comprobar_port() lo devuelve convertido a uint16_t. Si no,
     * cierra el programa.
     */
    comprobar_port(argv[1], &puerto);

    /********************** CREACIÓN DEL SOCKET ***************************/

    // Generamos un socket de dominio IPv4 y orientado a conexión (TCP)
    // El identificador se almacenará en sockserv
    sockserv = crear_socket();

    // Ponemos un tiempo límite de inactividad
    // Si pasan más de 30 segundos sin enviar datos, el socket se cierra
    poner_limite_inactividad(sockserv, (time_t) T_LIMITE);

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
     * Permitimos hasta MAX_COLA clientes esperando ser atendidos. En
     * consecuencia, podrá haber múltiples conexiones secuenciales seguidas.
     */
    marcar_pasivo(sockserv, (unsigned int) MAX_COLA);


    /*********************** ATENCIÓN A LOS CLIENTES ***********************/

    // Para poder controlar el cierre del socket del servidor, atendemos
    // un número de clientes fijo.
    while(1){

        /*
         * Llamamos a accept(). Nos quedamos esperando la solicitud de conexión
         * de algún cliente, y le atendemos cuando llega.
         * Guardamos su dirección (puerto e IP) en ipportcli.
         */
        sockcon = atender(sockserv, &ipportcli);

        /*
         * Convertimos la IP binaria del cliente a texto, con argumentos:
         *      - AF_INET, ya que estamos utilizando únicamente IPv4.
         *      - La dirección a convertir.
         *      - Un puntero a la cadena donde guardar el resultado.
         *      - El número de bytes disponibles en la cadena (la definimos
         *      de tamaño el máximo posible para una dirección IPv4).
         */
        if (inet_ntop(
                    AF_INET, (const void *) &ipportcli.sin_addr.s_addr,
                    &ipcli[0], (socklen_t) sizeof(ipcli)
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


        /******************** RECEPCIÓN Y ENVÍO DE DATOS *******************/

        // Comienza a haber variaciones con respecto a servidor.c

        // Recibimos todos los datos que se envíen a través del socket
        // Solo salimos del while por un error, por cierre del socket o si
        // el cliente ha enviado 0 bytes.
        while ((nbytes = recibir(sockcon, linea)) > 0){

            // Aunque el formato estándar para ssize_t es %zd, los sistemas
            // antiguos no lo soportan. Por seguridad, casteamos a long
            printf("Se han recibido %ld bytes\n", (long) nbytes);
            printf("\tMensaje: %s\n", linea);

            i = 0;
            // Procesamos los datos hasta encontrar el final de la cadena
            // (trabajamos suponiendo que siempre se incluye '\0')
            while (linea[i] != '\0'){
                // Sobreescribimos la cadena pasando cada carácter a mayúsculas
                // Si el carácter no admite la conversión, se deja intacto
                linea[i] = toupper(linea[i]);
                i++;
            }

            // El servidor también imprime el mensaje en mayúsculas, a fin de
            // que se pueda comprobar que ha funcionado correctamente
            printf("\tMensaje transformado: %s\n", linea);

            /*
             * Devolvemos el mensaje cambiado al cliente.
             * No nos hace falta enviar los N bytes que puede llegar a ocupar
             * el buffer. Dado que sabemos que hay un '\0', podemos cortar ahí
             * el envío.
             * strlen() no incluye el carácter nulo, por lo que lo sumamos
             * adicionalmente.
             * Si send() no logra enviar todos los bytes, repetimos el envío
             * con la parte del mensaje que aún no se haya transmitido.
             */
            i = 0;
            while ((nbytes = enviar_nbytes(sockcon, &linea[i],
                    strlen(&linea[i]) + 1)) != strlen(&linea[i]) + 1){
                i += nbytes;
            }

            // Indicamos cuántos bytes se han enviado. Nótese que \0'' no ocupa
            // 1 byte, sino que al ser un carácter literal, C lo interpreta
            // como un entero.
            printf("\tRealizado envío de %ld bytes.\n\n", (long) nbytes);

            // Podemos corroborar que el funcionamiento es correcto comparando
            // el número de bytes recibidos con el número de bytes transmitidos
        }

        printf("\n");


        /************************** CIERRE DE LA CONEXIÓN *******************/

        // Cerramos el socket de la conexión del cliente actual
        cerrar_socket(sockcon);
    }

    /*********************** CIERRE DEL SERVIDOR ****************************/

    /*
     * En realidad, esta parte nunca se llega a ejecutar dado que el bucle
     * while es infinito. El cierre de los recursos se transfiere al sistema
     * operativo. No obstante, se dejan estas líneas por si se cambiara
     * el bucle para que fuera finito o pudiera finalizarse.
     */

    // Tras atender a los clientes, cerramos el socket del servidor
    cerrar_socket(sockserv);

    // En este programa no se ha reservado memoria

    printf("Cerrando servidor...\n\n");
    exit(EXIT_SUCCESS);
}
