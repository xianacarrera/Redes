#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>     // Macro NI_MAXHOST, NI_MAXSERV
#include<unistd.h>    //getopt
#include<ctype.h>     // isprint
#include<string.h>    // memset


/*
 * Dudas:
 * 1) Se admiten IPv4, IPv6 en binario, hexadecimal... o solo texto?  ----> Solo texto
 * 2) Debemos priorizar reservar memoria (ver ejemplo inet_pton)? Qué pasa si
 * la propia función no encuentra espacio de memoria?
 * 3) A partir de sockaddr_ castear al tipo abstracto ---> OK si me funciona
 * 4) Mejor conversión de string a int? ----> Podemos usar atoi
 * 5) En port a service, supone alguna diferencia usar sockaddr_in con respecto a sockaddr_in6?
 }
 */

/* Devuelve:
 * 2 -> Dirección IPv4 correcta
 * 1 -> Dirección IPv6 correcta
 * 0 -> Error
 */
short check_IP(const char * ip, struct in_addr ** ipv4bin,
        struct in6_addr ** ipv6bin){
    short errorIPv4, errorIPv6;

    // Comprobamos que haya suficiente memoria para el resultado binario
    if ((*ipv4bin = (struct in_addr *) malloc(sizeof(struct in_addr))) == NULL){
        // TODO: gestión error
        fprintf(stderr, "Error reservando memoria\n");
        exit(EXIT_FAILURE);
    }

    // Comprobamos que el input sea una dirección válida
    // Al mismo tiempo, determinamos si es una dirección IPv4 o IPv6
    errorIPv4 = inet_pton(AF_INET, ip, (void *) *ipv4bin);

    // La direccion es correcta y está en formato IPv4
    // free(ipv4bin);
    if (errorIPv4 == 1) return 2;

    if ((*ipv6bin = (struct in6_addr *) malloc(sizeof(struct in6_addr))) == NULL){
        // TODO: gestión error
        fprintf(stderr, "Error reservando memoria\n");
        exit(EXIT_FAILURE);
    }

    errorIPv6 = inet_pton(AF_INET6, ip, (void *) *ipv6bin);

    if (errorIPv6 != 1){        // La dirección no es válida
        // TODO: gestión error
        fprintf(stderr, "Error en inet_pton\n");

        if (errorIPv6 == 0)
            fprintf(stderr, "Dirección %s incorrecta\n", ip);
        else if (errorIPv6 == -1)
            fprintf(stderr, "Campo AF incorrecto\n");
        else
            fprintf(stderr, "Error indefinido\n");

        return 0;
    }

    return 1;    // La dirección es correcta y está en formato IPv6
}

short setup_socket(const char * ip, struct sockaddr **addr){
    struct in_addr * ipv4bin;
    struct in6_addr * ipv6bin;

    // Estas dos se declaran aquí o más abajo (más eficiente vs convención?)
    struct sockaddr_in * addr_v4;
    struct sockaddr_in6 * addr_v6;

    switch (check_IP(ip, &ipv4bin, &ipv6bin)) {
        case 2:
        if ((addr_v4 = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in))) == NULL){
            printf("Error");
        }

            addr_v4->sin_family = AF_INET;
            addr_v4->sin_addr = *ipv4bin;
            // No conocemos el puerto -> addr_v4.sin_port nos es indiferente

            *addr = (struct sockaddr *) addr_v4;
            return 1;
        case 1:
        if ((addr_v6 = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6))) == NULL){
            printf("Error");
        }

            addr_v6->sin6_family = AF_INET6;
            addr_v6->sin6_addr = *ipv6bin;
            // El campo sin6_flowinfo no se usa
            // El campo sin6_port nos es indiferente al no conocer el puerto

            *addr = (struct sockaddr *) &addr_v6;
            return 1;
        case 0:
            // TODO: error
            fprintf(stderr, "Error\n");
            return 0;
        default:
            fprintf(stderr, "Error desconocido\n");
    }
    return 0;
}

void address_a_hostname(const char * ip){
    struct sockaddr * addr;
    char hostname[NI_MAXHOST]; // NI_MAXHOST=1025 (longitud máxima de un nombre)
    short error_setup_socket, error_getnameinfo;

    // Comprobamos que el formato del input sea correcto
    error_setup_socket = setup_socket(ip, &addr);

    // cast char[] a char *?
    // No tenemos flags -> Lo dejamos a 0
    // cast a const?
    error_getnameinfo = getnameinfo((const struct sockaddr *) addr,
            (socklen_t) sizeof(*addr),
            hostname, (socklen_t) sizeof(hostname),
            NULL, 0, 0);

    if (error_getnameinfo){
        fprintf(stderr, "Error bla bla\n");
    }

    printf("****************************************************************\n");

    // Sabemos que el campo sa_family de addr es AF_INET o AF_INET6 porque
    // la función getnameinfo no lo modifica (es const)
    printf("Dirección IPv%d %s: %s\n", addr->sa_family == AF_INET? 4 : 6,
            ip, hostname);
}

uint16_t es_formato_port_valido(const char * port){
    // No vamos a usar atoi, que no permitiría distinguir un puerto 0 de un error
    char * final_conversion;
    unsigned long port_numerico;
    uint16_t port_formateado;

    // Pasamos la cadena de texto a convertir, un puntero que guardará el punto
    // donde termine la conversión (el valor numérico) y la base del número, 10
    // string to unsigned long
    port_numerico = strtoul(port, &final_conversion, 10);
    printf("%p\n", port);
    printf("%p\n", final_conversion);
    printf("%ld\n", strlen(port));
    // final_conversion apunta al caracter nulo -> 1 más que el final de la cadena
    printf("%p\n", &(port[strlen(port) - 1]) + sizeof(char));
    if (final_conversion != &(port[strlen(port) - 1]) + sizeof(char)){
        // port debe estar totalmente compuesto por números para que la conversión
        // termine en el final de la cadena port
        fprintf(stderr, "El puerto introducido no es válido\n");
        exit(EXIT_FAILURE);
    }

    // TODO: comprobar overflow con errno

    if (port_numerico > 65535){
        fprintf(stderr, "El puerto introducido está fuera del rango válido\n");
        exit(EXIT_FAILURE);
    }

    port_formateado = (uint16_t) port_numerico;
    return port_formateado;
}

void port_a_service(const char * port){
    // Comprobamos que el input sea numérico
    uint16_t port_num;
    int error;
    struct sockaddr_in addr;
    char service[NI_MAXSERV];  //NI_MAXSERV se define como la longitud máxima de la cadena de texto que representa un servicio

    port_num = es_formato_port_valido(port);

    // Construimos el socket
    // No podemos utilizar la estructura abstracta directamente
    // Construimos el socket con IPv4 ¿¿¿DA IGUAl??
    // Hace falta sin_family??
    addr.sin_port = htons(port_num);
    addr.sin_family = AF_INET;

    // Casteamos el socket a la estructura abstracta sockaddr
    error = getnameinfo((const struct sockaddr *) &addr,
            (socklen_t) sizeof(addr), NULL, 0, service, NI_MAXSERV, 0);

    if (error){
        fprintf(stderr, "Error en la función getnameinfo de port a service\n");
        exit(EXIT_FAILURE);
    }

    printf("****************************************************************\n");
    printf("Puerto: %s: servicio %s\n", port, service);
}

void service_a_port(const char * service){
    struct addrinfo *res;
    struct addrinfo hints;
    uint16_t raw_port, port;
    int error;

    // ?????????? HINTS??????????
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;

    error = getaddrinfo(NULL, service,
            (const struct addrinfo *) &hints, &res);
    if (error){
        fprintf(stderr, "Error en getaddrinfo de service_a_port\n");
        exit(EXIT_FAILURE);
    }

    // res tendrá únicamente una estructura addrinfo
    if (res->ai_family == AF_INET){
        raw_port = ((struct sockaddr_in *) res->ai_addr)->sin_port;
    } else if (res->ai_family == AF_INET6){
        raw_port = ((struct sockaddr_in6 *) res->ai_addr)->sin6_port;
    } else {
        printf("Error");
        return;
    }

    // El puerto está en orden de red (big-endian). Tenemos que convertirlo a orden de host (puede ser big-endian o little-endian según el host)
    port = ntohs(raw_port);

    // El formato de impresión definido para uint16_t es PRId16
    printf("****************************************************************\n");
    printf("Servicio %s: puerto %" PRId16 "\n", service, port);
}

void name_a_hostinfo(const char * name){
    struct addrinfo *res;
    struct addrinfo *rp;    // Puntero para avanzar por la lista enlazada
    struct addrinfo hints;
    struct sockaddr_in * socka_v4;
    struct sockaddr_in6 * socka_v6;
    struct in_addr * ip_v4;
    struct in6_addr * ip_v6;
    char * iptext = NULL;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;      // Innecesario?
    hints.ai_protocol = 0;      // Innecesario?
    hints.ai_flags = AI_CANONNAME;

    error = getaddrinfo(name, NULL, (const struct addrinfo *) &hints, &res);
    if (error){
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    printf("Nombre canónico: %s\n", res->ai_canonname);
    for (rp = res; rp != NULL; rp = rp->ai_next){
        if (rp->ai_family == AF_INET){
            // TODO: eliminar socka_v4???
            socka_v4 = (struct sockaddr_in *) rp->ai_addr;
            ip_v4 = &(socka_v4->sin_addr);
            iptext = (char *) malloc(rp->ai_addrlen);
        } else if (rp->ai_family == AF_INET6){
            socka_v6 = (struct sockaddr_in6 *) rp->ai_addr;
            ip_v6 = &(socka_v6->sin6_addr);
            iptext = (char *) malloc(rp->ai_addrlen);
        } else {
            printf("Error");
            return;
        }

        if ((inet_ntop(
                rp->ai_family,
                rp->ai_family == AF_INET? (const void *) ip_v4 : (const void *) ip_v6,
                iptext,
                rp->ai_addrlen
        )) != NULL){
            printf("Dirección IPv%d: %s\n",
                    rp->ai_family == AF_INET? 4: 6, iptext);
            if (iptext != NULL) free(iptext);
            iptext = NULL;
            continue;
        }

        /* LOS RESULTADOS SALEN REPETIDOS PORQUE LAS VARIACIONES ESTÁN EN DETALLES
        * QUE NO SE IMPRIMEN, COMO EL PROTOCOLO O EL TIPO DE SOCKET.
        * RESTRINGIR RESULTADO O IMPRIMIR MÁS INFORMACIÓN???
        */

        printf("Error 2\n");
        if (iptext != NULL) free(iptext);
        iptext = NULL;
    }

    freeaddrinfo(res);
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
     name_a_hostinfo(name);
  if (service)
     // Llamar a la funcion para obtener informacion del servicio
     service_a_port(service);
  if (addr)
     // Llamar a la funcion para obtener informacion de la IP
     address_a_hostname(addr);
  if (port)
     // Llamar a la funcion para obtener informacion del puerto
     port_a_service(port);

  printf("****************************************************************\n\n");

  // Finalizamos correctamente, con codigo de salida EXIT_SUCCESS
  return (EXIT_SUCCESS);
}
