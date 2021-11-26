#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h>
#include "lib.h"


// Comprobación local -> Puertos distintos. Usar todo el rango de locales???

#define MAX_COLA 3       // Número máximo de clientes esperando ser atendidos
#define T_LIMITE 30      // Número de segundos que el socket puede permanecer
                         // inactivo.


int main(int argc, char * argv[]){
    int socket;
    uint16_t puerto_propio;
    uint16_t puerto_remoto;
    struct sockaddr_in dir_remota;
    char mensaje[] = "Is this the real life?";
    ssize_t nbytes;                  // Número de bytes enviados
    float fibonnacci[] = {1.1, 1.1, 2.2, 3.3, 5.5, 8.8, 14.3, 23.1};


    /************************ COMPROBACIÓN DEL INPUT ***********************/

    // El usuario debe introducir el puerto desde el que el servidor
    // escuchará las peticiones de los clientes
    if (argc != 4)
        // Imprimimos un error sin llamar a perror() (0 como segundo
        // argumento y cerramos el programa
        cerrar_con_error("Son nesarios 3 argumentos:\n"
                "\tPuerto propio (desde donde se enviarán datos)\n"
                "\tIP remota (IP del equipo del programa 2)\n"
                "\tPuerto remoto (puerto del programa 2)", 0);

    /*
     * Si el puerto no es válido, comprobar_port() finaliza la ejecución.
     * Se comrpueba también que el puerto sea mayor que IPPORT_USERRESERVED.
     * Si es válido, lo convierte a un uint16_t y guarda el resultado en puerto
     */
    comprobar_port(argv[1], &puerto_propio);

    // Comprobamos la IP con inet_pton
    memset(&dir_remota, 0, sizeof(dir_remota));
    // Lo introducimos directamente en la estructura sockaddr_in
    if ((inet_pton(AF_INET, (const char *) argv[2],
            (void *) &dir_remota.sin_addr)) < 0)
        cerrar_con_error("Error - la IP debe tener un formato IPv4 válido", 0);

    // Si la IP es la local (127.0.0.1, los puertos tienen que ser distintos)

    if (!strcmp(argv[2], "127.0.0.1") && !strcmp(argv[1], argv[3]))
        cerrar_con_error("Si se usa la IP de loopback, los puertos tienen "
                "que ser distintos", 0);


        //jcklsfjdsklfjdslkfjslkfjslkfjslk
    comprobar_port(argv[3], &puerto_remoto);

    /************************* CREACIÓN DEL SOCKET *************************/

    // Generamos un socket de dominio IPv4 y orientado a conexión (TCP)
    // sockserv guardará el entero identificador del socket
    socket = crear_socket();

    // Ponemos un tiempo límite de inactividad
    // Si pasan más de 30 segundos sin enviar datos, el socket se cierra
    //poner_limite_inactividad(socket, (time_t) T_LIMITE);

    /********************* ASIGNACIONES DE DIRECCIONES *********************/

    /*
     * Asignamos una dirección IP y un puerto al socket que acabamos de crear.
     * Como IP elegimos INADDR_ANY (escuchará a través de cualquier interfaz).
     * Como puerto elegimos el indicado por el usuario.
     */
    asignar_direccion_socket(socket, puerto_propio);

    dir_remota.sin_family = AF_INET;
    dir_remota.sin_port = htons(puerto_remoto);


    /**************************** ENVÍO DE DATOS ***************************/

    //nbytes = enviar(socket, (void *) mensaje, &dir_remota);
    nbytes = enviar(socket, (void *) fibonnacci, &dir_remota);

    printf("Se han enviado %ld bytes\n", (long) nbytes);
    printf("%ld\n", sizeof(fibonnacci));
    //printf("Mensaje enviado: %s\n", mensaje);



    /*********************** CIERRE DEL SOCKET ****************************/

    // Tras enviar los datos, cerramos el socket
    cerrar_socket(socket);

    // En este programa no se ha reservado memoria

    printf("Cerrando programa1...\n\n");
    exit(EXIT_SUCCESS);
}
