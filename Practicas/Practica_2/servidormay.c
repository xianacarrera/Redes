#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include "lib.h"

/*
 * TODO:
 * perror
 * LÍMITE DEL PUERTO
 * Se puede usar inet_addr????
 * Podemos meter las funciones en una librería?

 */

int main(int argc, char * argv[]){
    uint16_t puerto;
    int sockserv;           // Socket de servidor
    int sockcon;            // Socket de conexión
    struct sockaddr_in ipportcli;
    socklen_t length_ptr;
    char linea[N];
    char ipcli[INET_ADDRSTRLEN];
    int nbytes;
    //int leido;
    int i;

    if (argc != 2)
        cerrar_con_error("Indicar un argumento: el puerto del servidor", 0);

    if (!comprobar_port(argv[1], &puerto))
        exit(EXIT_FAILURE);

    /********************** CREACIÓN DEL SOCKET ***************************/
    sockserv = crear_socket();

    /******************* ASIGNACIÓN DE DIRECCIÓN ***************************/
    asignar_direccion_puerto(sockserv, puerto);

    /************************** ESCUCHA *******************************/
    // Ponemos el servidor a la escucha
    // Permitimos que pueda tener hasta 5 clientes a la cola
    marcar_pasivo(sockserv, 5);

    while(1){

        /********************* ATENCIÓN A LAS CONEXIONES ********************/
        /*length_ptr = sizeof(struct sockaddr_in);
        if ((sockcon = accept(
                    sockserv, (struct sockaddr *) &ipportcli, &length_ptr
                )) < 0){
            perror("Error - no se pudo aceptar la conexión con accept()");
            exit(EXIT_FAILURE);
        }*/
        sockcon = atender(sockserv, &ipportcli, &length_ptr);

        // length_ptr como 4º argumento???
        if (inet_ntop(
                    AF_INET, (const void *) &ipportcli.sin_addr.s_addr,
                    &ipcli[0], (socklen_t) length_ptr
                ) == NULL)
            cerrar_con_error("Error al convertir la IP binaria del cliente "
                    "a texto", 1);

        printf("Conexión con la IP %s y el puerto %d\n",
                ipcli, ntohs(ipportcli.sin_port));

        // El puerto es distinto al del servidor????

        /*************************** ENVÍO DE DATOS *************************/

        while ((nbytes = recibir(sockcon, linea)) > 0){

            // Como fgets puede leer '\0' sin problema, no paramos al encontrarnos
            // uno, sino al procesar todos los bytes


            printf("\nMensaje de %d bytes recibido:\n", nbytes);
            /*
            leido = 0;
            while (leido < nbytes){
                printf("%c", linea[leido]);
                linea[leido] = toupper(linea[leido]);
                leido++;
            }*/
            i = 0;
            while (linea[i] != '\n'){
                printf("%c", linea[i]);
                linea[i] = toupper(linea[i]);
                i++;
            }


            // Enviar N bytes???
            // i + 1 por el carácter nulo?
            nbytes = enviar_nbytes(sockcon, linea, i + 1);

            printf("\nRealizado envío de %d byte%s.\n", nbytes,
                nbytes > 1? "s" : "");
            /*
            printf("Longitud del mensaje original = %ld bytes\n",
                    strlen(mensaje) + sizeof('\0'));
            */
        }

        printf("\n\n");

        /************************** CIERRE DE LA CONEXIÓN *******************/

        cerrar_socket(sockcon);
    }
}
