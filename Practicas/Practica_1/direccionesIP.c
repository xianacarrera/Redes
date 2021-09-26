#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int nametohost(char * name){
    struct addrinfo *res;
    struct addrinfo *rp;
    struct addrinfo hints;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Direcciones IPv4 e IPv6
    hints.ai_socketype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_CANONNAME;

    /*
     * Argumentos:
     * const char * -> cadena con el nombre del host a convertir
     * const char * -> cadena con el nombre del servicio a convertir
     * const struct addrinfo * -> struct addrinfo con especificaciones sobre
     *             las características que debe tener el resultado
     * struct addrinfo ** -> lista enlazada de estructuras addrinfo donde se
     *             guardará el resultado
     */
    error = getaddrinfo(const name, NULL, &hints, &res);

    if (error != 0){
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    printf("Nombre canónico: %s\n", res->ai_canonname);
    for (rp = res; rp != NULL; rp = rp->ai_next){
        // rp comienza en la dirección de res, que corresponde al primer elemento
        // y va recorriendo la lista enlazada. Cada elemento de res es un puntero a Nunca/// estructura
    }

}


int main(int argc, char * argv[]) {
  int opt;
  // name: Nombre del host
  // service: Nombre del servicio
  // addr: Direccion IPv4 o IPv6
  // port: Puerto
  char * name = NULL, * service = NULL, * addr = NULL, * port = NULL;

  // Comprueba que exista al menos un operando
  // En caso de error salimos de la función main con el codigo EXIT_FAILURE
  if (argc < 2) {
    printf("Falta un operando\n");
    printf("Usar: %s [-n Nombre del host] [-s Nombre del servicio (p.e. http)] [-i Direccion ip] [-p Numero de puerto]\n", argv[0]);
    return (EXIT_FAILURE);
  }

  // La funcion getopt() permite de forma facil manejar operandos en linea de comandos
  // Las opciones n: s: i: p: indican que esos "flags" (nsip) deben de ir seguidos de un argumento
  // Ese parametro se guarda en la variable externa optarg
  while ((opt = getopt(argc, argv, ":n:s:i:p:")) != -1) {
    switch (opt) {
    case 'n':
      name = optarg; // Argumento nombre de host
      break;
    case 's':
      service = optarg; // Argumento nombre de servicio
      break;
    case 'i':
      addr = optarg; // Argumento direccion ip
      break;
    case 'p':
      port = optarg; // Argumento numero de puerto
      break;
    case ':': // Se intrudujo un flag sin argumento obligatorio
      fprintf(stderr, "La opción -%c requiere un argumento.\n", optopt);
      return (EXIT_FAILURE);
      break;
    case '?':
      if (isprint(optopt)) // Se introdujo un flag incorrecto
        fprintf(stderr, "Opción desconocida `-%c'.\n", optopt);
      else // Hay un caracter no legible en las opciones
        fprintf(stderr, "Caracter de opción desconocido `\\x%x'.\n", optopt);
      return (EXIT_FAILURE);
      break;
    default: // Se produjo un error indeterminado. Nunca se deberia llegar aqui.
      abort();
    }
  }

  printf("\n");
  // Llamamos a las funciones correspondientes a los argumentos solicitados (que son != NULL)
  if (name)
     // Llamar a la funcion para obtener informacion del host
  if (service)
     // Llamar a la funcion para obtener informacion del servicio
  if (addr)
     // Llamar a la funcion para obtener informacion de la IP
  if (port)
     // Llamar a la funcion para obtener informacion del puerto

  printf("****************************************************************\n\n");

  // Finalizamos correctamente, con codigo de salida EXIT_SUCCESS
  return (EXIT_SUCCESS);
}
