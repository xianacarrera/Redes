#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include "lib.h"

// Comprobar fichero vacío

int main(int argc, char * argv[]){
    FILE *fichero_ent;
    FILE *fichero_sal;
    char * nombre_fichero_sal;
    uint16_t puerto;
    int sockserv;
    struct sockaddr_in direccion;
    char buffer[N];
    char linea[N];
    char * frase;
    int res_recv;
    //int leido = 0;
    int leido;
    int nbytes;
    int i;

    if (argc != 4)
        cerrar_con_error("Introducir 3 argumentos: fichero de entrada, IP y "
                "puerto del servidor", 0);

    if ((fichero_ent = fopen(argv[1], "r")) == NULL)
        cerrar_con_error("No se pudo abrir el fichero de entrada", 1);

    if ((nombre_fichero_sal =
            (char *) malloc((strlen(argv[1]) + 1) * sizeof(char))) == NULL)
        cerrar_con_error("Error reservando memoria", 0);

    for (i = 0; i < strlen(argv[1]); i++)
        *(nombre_fichero_sal + i) = toupper(*(argv[1] + i));
    *(nombre_fichero_sal + i) = '\0';

    if ((fichero_sal = fopen(nombre_fichero_sal, "w")) == NULL){
        fprintf(stderr, "Sobreescritura/creación de %s fallida",
                nombre_fichero_sal);
        perror("Error");
    }

    if (!comprobar_port(argv[3], &puerto))
        exit(EXIT_FAILURE);

    // Comprobamos la IP con inet_pton
    memset(&direccion, 0, sizeof(direccion));
    // Lo introducimos directamente en la estructura sockaddr_in
    if ((inet_pton(AF_INET, argv[2], (void *) &direccion.sin_addr)) < 0)
        cerrar_con_error("Error - la IP debe tener un formato IPv4 válido", 0);

    sockserv = crear_socket();


    solicitar_conexion(sockserv, direccion, puerto);

    // fgets lee a lo sumo 1 carácter menos que lo indicado como 2º argumento
    // Devuelve NULL si ha tenido lugar un error o ha llegado al final del fichero
    // Podemos distinguir entre esas situaciones usando ferror() o feof()
    if (fgets(linea, N, fichero_ent) == NULL && ferror(fichero_ent))
        cerrar_con_error("Ha tenido lugar un error en la lectura del "
                "fichero", 0);

    // Nótese que fgets puede leer '\0'. No para en ese punto.

    // Enviar todos los bytes???
    nbytes = enviar_nbytes(sockserv, linea, N);

    printf("Se han enviado %d bytes.\n", nbytes);
    /*
    printf("Longitud del mensaje original = %ld bytes\n",
            strlen(linea) + sizeof('\0'));
*/

    res_recv = recibir(sockserv, &buffer[0]);
    printf("Mensaje de %d bytes recibido:\n", res_recv);
    leido = 0;
    while (leido < res_recv){
        frase = &buffer[leido];
        printf("%s", frase);
        leido += strlen(frase) + sizeof('\0');
    }


    cerrar_socket(sockserv);

    exit(EXIT_SUCCESS);
}
