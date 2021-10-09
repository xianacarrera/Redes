#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>     // Macro NI_MAXHOST, NI_MAXSERV
#include<unistd.h>    //getopt
#include<ctype.h>     // isprint
#include<string.h>    // memset
#include<limits.h>    // ULONG_MAX
#include<errno.h>     // Overflow

/*
 * Dudas:
 * 1) Se admiten IPv4, IPv6 en binario, hexadecimal... o solo texto?  ----> Solo texto
 * 2) Debemos priorizar reservar memoria (ver ejemplo inet_pton)? Qué pasa si
 * la propia función no encuentra espacio de memoria?
 * 3) A partir de sockaddr_ castear al tipo abstracto ---> OK si me funciona
 * 4) Mejor conversión de string a int? ----> Podemos usar atoi
 * 5) En port a service, supone alguna diferencia usar sockaddr_in con respecto a sockaddr_in6?
 * 6) EXIT???
  7) Hacer return en fallos no críticos???
  8) Función pton??
  9) Cambiar a 4 funciones --> PREGUNTAR
 */

void name_a_hostinfo(const char * name){
    struct addrinfo *res;     // Lista enlazada que guardará el resultado
    struct addrinfo *rp;      // Puntero para avanzar por el resultado
    struct addrinfo hints;    // Configuración de opciones de getaddrinfo
    struct sockaddr_in * socka_v4;   // Encapsula socket para direcciones IPv4
    struct sockaddr_in6 * socka_v6;  // Encapsula socket para direcciones IPv6
    struct in_addr * ip_v4;       // Encapsula una dirección IPv4 en binario
    struct in6_addr * ip_v6;      // Encapsula una dirección IPv6 en binario
    char * iptext;                // Direcciones IP en formato textual
    int error;                    // Gestión de errores de getaddrinfo

    printf("*********************************************************"
            "*******\n");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Obtenemos direcciones IPv4 e IPv6
    hints.ai_socktype = SOCK_STREAM;    // Para no recibir varias copias de
    // la misma IP por las distintas combinaciones IP/tipo de socket,
    // filtramos solo una IP de cada tipo seleccionando SOCK_STREAM.
    hints.ai_flags = AI_CANONNAME;      // El nombre canónico del dominio
    // se guardará en el campo ai_canonname del primer elemento del resultado

    // Admitiremos todos los protocolos. Por tanto, utilizaremos
    // ai_protocol = 0. Nótese que este campo ya fue puesto a 0 con memtset.


    // Llamamos a getaddrinfo indicando el nombre a convertir
    // No pedimos ningún servicio en particular (NULL)
    // Pasamos las opciones seleccionadas como puntero a hints
    // El resultado se guardará en res
    error = getaddrinfo(name, NULL, (const struct addrinfo *) &hints, &res);

    if (error){
        // gai_strerror describe el error en función del código recibido
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(error));
        return;
    }

    // Accedemos al nombre canónico a través de la primera estructura
    printf("Nombre canónico: %s\n", res->ai_canonname);

    // Recorremos la lista enlazada de estructuras
    for (rp = res; rp != NULL; rp = rp->ai_next){
        if (rp->ai_family == AF_INET){              // Dirección IPv4
            socka_v4 = (struct sockaddr_in *) rp->ai_addr;
            ip_v4 = &(socka_v4->sin_addr);          // Guardamos la dirección

            // Reservamos espacio para la dirección textual
            // La longitud máxima será INET_ADDRSTRLEN al ser una IP_v4
            if ((iptext = (char *) malloc(INET_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria\n");
                break;
            }

        } else if (rp->ai_family == AF_INET6){     // Dirección IPv6
            socka_v6 = (struct sockaddr_in6 *) rp->ai_addr;
            ip_v6 = &(socka_v6->sin6_addr);        // Guardamos la dirección

            // Reservamos espacio para la dirección textual
            // La longitud máxima será INET6_ADDRSTRLEN al ser una IP_v6
            if ((iptext = (char *) malloc(INET6_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria\n");
                break;
            }

        } else {      // No se debería dar nunca
            fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido "
                    "con getaddrinfo\n");
            // Dado que desconocemos si el resto de resultados son correctos,
            // cortamos la ejecución
            break;
        }

        /*
        * Convertimos la dirección obtenida de binario a textual
        * Pasamos como argumento:
        *     1) La familia de la dirección (AF_INET o AF_INET6)
        *     2) Un puntero a la estructura con la dirección en binario
        *     3) Un puntero a la cadena que guardará el resultado
        *     4) El tamaño en bytes de la cadena de destino. socklen_t es un
        *        entero de al menos 32 bits.
        */
        if ((inet_ntop(
                rp->ai_family,
                rp->ai_family == AF_INET?
                    (const void *) ip_v4 : (const void *) ip_v6,
                iptext,
                rp->ai_family == AF_INET? INET_ADDRSTRLEN : INET6_ADDRSTRLEN
        )) == NULL){
            fprintf(stderr, "Error en la conversión de binario a textual con "
                    "inet_ntop\n");
            break;      // Cortamos la ejecución
        }

        // No se han encontrado errores
        printf("Dirección IPv%d: %s\n", rp->ai_family == AF_INET? 4:6, iptext);

        free(iptext);    // Liberamos el puntero para poder reutilizarlo
        iptext = NULL;
    }

    freeaddrinfo(res);    // Liberamos la lista enlazada
    // rp, que apuntaba a una estructura de la lista (o a NULL, si la
    // recorrió hasta el final), también queda liberado
    // Análogo para las direcciones y sockets encapsulados

    // En caso de error, puede que iptext aún no se haya liberado
    if (iptext != NULL) free(iptext);
}

void service_a_port(const char * service){
    struct addrinfo *res;          // Guardará el resultado de getaddrinfo
    struct addrinfo hints;         // Configuración de opciones de getaddrinfo
    uint16_t port_or;              // Puerto obtenido en orden de red
    int error;                     // Gestión de errores de getaddrinfo

    printf("*********************************************************"
            "*******\n");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // Nos es indiferente seleccionar IPv4 o IPv6

    // Los campos ai_socktype, ai_protocol y ai_flags se mantienen a 0,
    // indicando que se acepta cualquier tipo de socket, cualquier protocolo,
    // y sin seleccionar opciones adicionales, respectivamente.

    /*
     * Llamamos a getaddrinfo:
     *      - sin indicar nombre de host (lo dejamos a NULL)
     *      - pasando la cadena con el servicio a traducir en puerto
     *      - pasando las opciones seleccionadas (puntero a hints)
     *      - pasando un puntero a la lista enlazada que guardará el resultado
     */
    error = getaddrinfo(NULL, service, (const struct addrinfo *) &hints, &res);

    if (error){
        // gai_strerror describe el error en función del código recibido
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(error));
        return;
    }

    // El resultado contendrá una única estructura addrinfo
    if (res->ai_family == AF_INET){
        port_or = ((struct sockaddr_in *) res->ai_addr)->sin_port;
    } else if (res->ai_family == AF_INET6){
        port_or = ((struct sockaddr_in6 *) res->ai_addr)->sin6_port;
    } else {
        fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido en "
                "getaddrinfo\n");
        return;
    }


    printf("Servicio %s: puerto %" PRId16 "\n", service, ntohs(port_or));
    /*
     * El puerto está en orden de red (big-endian). Para imprimirlo al usuario,
     * utilizamos ntohs para convertirlo a orden de host (puede ser big-endian
     * o little endian dependiendo del sistema).
     *
     * El formato de impresión definido para uint16_t es PRId16.
     */

     // Liberamos la memoria reservada para la lista enlazada
     freeaddrinfo(res);
}

/* Devuelve:
 * 2 -> Dirección IPv4 correcta
 * 1 -> Dirección IPv6 correcta
 * 0 -> Error

 // Comprobamos que el input sea una dirección válida
 // Al mismo tiempo, determinamos si es una dirección IPv4 o IPv6
 */
short check_IP(const char * ip, struct in_addr * ipv4bin,
        struct in6_addr * ipv6bin){

    // Intentamos convertir la IP a formato binario suponiendo que es IPv4
    // El último argumento es un puntero a void donde se guardará el resultado
    // Si hay éxito (devuelve 1), la dirección era una IPv4 correcta
    if (inet_pton(AF_INET, ip, (void *) ipv4bin) == 1)
        return 2;     // Resultado: dirección IPv4

    // Intentamos convertir la IP en formato binario suponiendo que es IPv6
    // El resultado queda almacenado en la dirección a la que apunta ipv6bin
    switch (inet_pton(AF_INET6, ip, (void *) ipv6bin)) {
        case 1: return 1;    // Dirección IPv6 válida
        case 0:
            fprintf(stderr, "La dirección indicada no tiene ni formato IPv4 "
                    "ni formato IPv6 válidos\n");
            break;
        case -1:
            fprintf(stderr, "La familia de direcciones pasada como argumento "
                    "(AF) es incorrecta\n");
            break;
        default:
            fprintf(stderr, "Error desconocido en inet_pton\n");
    }

    return 0;     // Resultado de error
}

short setup_socket(const char * ip, struct sockaddr **addr){
    struct in_addr ipv4bin;         // Encapsula una dirección IPv4 binaria
    struct in6_addr ipv6bin;        // Encapsula una dirección IPv6 binaria
    struct sockaddr_in * addr_v4;   // Encapsulará la dirección IPv4
    struct sockaddr_in6 * addr_v6;  // Encapsulará la dirección IPv6

    /*
     * Llamamos a check_IP para comprobar si la IP es válida.
     * Si es así, nos la guarda en formato binario en ipv4bin o ipv6bin,
     * según corresponda.
     */
    switch (check_IP(ip, &ipv4bin, &ipv6bin)) {
        case 2:        // Dirección IPv4 válida
            // Reservamos memoria para el sockaddr_in
            if ((addr_v4 = (struct sockaddr_in *)
                        malloc(sizeof(struct sockaddr_in))
                ) == NULL){
                fprintf(stderr, "Error reservando memoria\n");
                return 0;    // Error
            }

            addr_v4->sin_family = AF_INET;    // Es una IPv4
            addr_v4->sin_addr = ipv4bin;     // Asignamos directamente la IP
            // Como no conocemos el puerto, no utilizamos el campo sin_port

            // El cast de sockaddr_in a sockaddr está soportado
            *addr = (struct sockaddr *) addr_v4;   // Damos la referencia
            return 1;     // Éxito

        case 1:
            if ((addr_v6 = (struct sockaddr_in6 *)
                    malloc(sizeof(struct sockaddr_in6))
                ) == NULL){
                fprintf(stderr, "Error reservando memoria\n");
                return 0;     // Error
            }

            addr_v6->sin6_family = AF_INET6;   // Es una IPv6
            addr_v6->sin6_addr = ipv6bin;     // Asignamos directamente la IP
            // El campo sin6_flowinfo no se usa
            // El campo sin6_port nos es indiferente al no conocer el puerto

            // El cast de sockaddr_in6 a sockaddr está soportado
            *addr = (struct sockaddr *) &addr_v6;    // Damos la referencia
            return 1;      // Éxito

        case 0:
            // Hubo un error en check_IP (la propia función lo imprime)
            return 0;
        default:        // Nunca debería darse
            fprintf(stderr, "Error no documentado al comprobar la IP\n");
    }
    return 0;
}

void address_a_hostname(const char * ip){
    struct sockaddr * addr;
    char hostname[NI_MAXHOST]; // NI_MAXHOST es (longitud máxima de un nombre)
    short error;

    printf("*********************************************************"
            "*******\n");

    /*
     * Llamamos a setup_socket para comprobar que el input tenga el formato
     * correcto y, a la vez, preparar la estructura de la dirección a convertir
     */
    if (!setup_socket(ip, &addr)) return;

    /*
     * Llamamos a getnameinfo con los siguientes argumentos:
     *      1) addr, la dirección a convertir
     *      2) el tamaño de la estructura addr (como socklen_t, un entero de
     *         al menos 32 bits)
     *      3) la cadena donde se guardará el nombre del host
     *      4) la longitud máxima de la cadena anterior
     *      5) como no queremos información de servicios, NULL
     *      6) dado que no recibiremos el nombre de ningún servicio, la
     *         longitud máxima de su cadena será 0
     *      7) no necesitamos flags, por lo que dejamos el argumento a 0
     */
    error = getnameinfo((const struct sockaddr *) addr,
            (socklen_t) sizeof(*addr), hostname, (socklen_t) sizeof(hostname),
            NULL, 0, 0);

    if (error){    // La función devuelve un valor distinto de 0
        // Mostramos el significado del código de error con gai_strerror()
        fprintf(stderr, "Error en getnameinfo: %s\n",
                gai_strerror(error_getnameinfo));
        return;
    }

    printf("Dirección IPv%d %s: %s\n", addr->sa_family == AF_INET? 4 : 6,
            ip, hostname);
    // Sabemos que el campo sa_family de addr es AF_INET o AF_INET6 porque
    // la función getnameinfo no lo modifica (es const)

    // Liberamos la memoria que se reservó para addr en setup_socket
    free(addr);
}

// Devuelve 0 si error, 1 si bien
short es_formato_port_valido(const char * port, uint16_t * port_formateado){
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
        fprintf(stderr, "El puerto introducido no es válido\n");
        return 0;
    }

    // Comprobamos que no haya tenido lugar un overflow
    if (port_numerico == ULONG_MAX && errno == ERANGE){
        fprintf(stderr, "El puerto introducido está fuera del rango "
                "representable\n");
        perror("Error del sistema: ");   // Imprimos el error que da errno
        return 0;
    }

    if (port_numerico > 65535){
        fprintf(stderr, "El puerto introducido está fuera del rango "
                "válido para una IP: [0, 65535]\n");
        return 0;
    }

    *port_formateado = (uint16_t) port_numerico;
    return 1;
}

void port_a_service(const char * port){
    uint16_t port_num;
    int error;
    struct sockaddr_in addr;
    char service[NI_MAXSERV];  //NI_MAXSERV se define como la longitud máxima de la cadena de texto que representa un servicio

    printf("*********************************************************"
            "*******\n");

    // Comprobamos que el input sea numérico y esté dentro del rango límite
    if (!es_formato_port_valido(port, &port_num)) return;

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

    printf("Puerto: %s: servicio %s\n", port, service);
}

/*
 * Obtiene el nombre canónico y direcciones IP asociadas a un nombre de
 * host, así como el puerto asociado a un servicio.
 *
 * Esta función combina las funcionalidades de name_a_hostinfo y
 * service_a_port, ofreciendo una solución más eficiente para el caso
 * combinado en el que el usuario da un nombre y un servicio.
 */
void name_service_a_hostinfo_port(const char * name, const char * service){
    struct addrinfo *res;     // Lista enlazada que guardará el resultado
    struct addrinfo *rp;      // Puntero para avanzar por el resultado
    struct addrinfo hints;    // Configuración de opciones de getaddrinfo
    struct sockaddr_in * socka_v4;   // Encapsula socket para direcciones IPv4
    struct sockaddr_in6 * socka_v6;  // Encapsula socket para direcciones IPv6
    struct in_addr * ip_v4;       // Encapsula una dirección IPv4 en binario
    struct in6_addr * ip_v6;      // Encapsula una dirección IPv6 en binario
    char * iptext;                // Direcciones IP en formato textual
    uint16_t port_or;             // Puerto obtenido en orden de red
    int error;                    // Gestión de errores de getaddrinfo

    printf("********************************************************"
            "********\n");

    // La configuración de las opciones es análoga a la de name_a_hostinfo:
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Obtenemos direcciones IPv4 e IPv6
    hints.ai_socktype = SOCK_STREAM;    // Para no recibir varias copias de IPs
    hints.ai_flags = AI_CANONNAME;      // El nombre canónico del dominio
    // se guardará en el campo ai_canonname del primer elemento del resultado

    // Admitimos cualquier protocolo. Por tanto, dejamos ai_protocol a 0.


    // Llamamos a getaddrinfo indicando el nombre a convertir
    // Indicamos además el servicio dado por el usuario como 2º argumento
    // Pasamos las opciones seleccionadas como puntero a hints
    // El resultado se guardará en res
    error = getaddrinfo(name, service, (const struct addrinfo *) &hints, &res);

    if (error){
        // gai_strerror describe el error en función del código recibido
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(error));
        return;
    }

    // Podemos acceder al nombre canónico y al puerto desde la 1ª estructura
    // (El puerto se obtiene con el servicio, no depende de la estructura)
    printf("Nombre canónico: %s\n", res->ai_canonname);
    if (res->ai_family == AF_INET){           // Puerto configurado para IPv4
        port_or = ((struct sockaddr_in *) res->ai_addr)->sin_port;
    } else if (res->ai_family == AF_INET6){   // Puerto configurado para IPv6
        port_or = ((struct sockaddr_in6 *) res->ai_addr)->sin6_port;
    } else {
        fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido en "
                "getaddrinfo\n");
        return;
    }

    // Recorremos la lista enlazada de estructuras
    for (rp = res; rp != NULL; rp = rp->ai_next){
        if (rp->ai_family == AF_INET){              // Dirección IPv4
            socka_v4 = (struct sockaddr_in *) rp->ai_addr;
            ip_v4 = &(socka_v4->sin_addr);          // Guardamos la dirección

            // Reservamos espacio para la dirección en formato textual
            // La longitud máxima será INET_ADDRSTRLEN (es una IP_v4)
            if ((iptext = (char *) malloc(INET_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria\n");
                break;
            }

        } else if (rp->ai_family == AF_INET6){     // Dirección IPv6
            socka_v6 = (struct sockaddr_in6 *) rp->ai_addr;
            ip_v6 = &(socka_v6->sin6_addr);        // Guardamos la dirección

            // Reservamos espacio para la dirección en formato textual
            // La longitud máxima será INET6_ADDRSTRLEN (es una IP_v6)
            if ((iptext = (char *) malloc(INET6_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria\n");
                break;
            }

        } else {      // Nunca se debería dar
            fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido "
                    "con getaddrinfo\n");
            // Paramos la ejecución al no saber si el resto de resultados
            // serán correctos o no
            break;
        }

        /*
        * Convertimos la dirección obtenida de binario a textual
        * Pasamos como argumento:
        *     1) La familia de la dirección (AF_INET o AF_INET6)
        *     2) Un puntero a la estructura con la dirección en binario
        *     3) Un puntero a la cadena que guardará el resultado
        *     4) El tamaño en bytes de la cadena de destino. socklen_t es un
        *        entero de al menos 32 bits.
        */
        if ((inet_ntop(
                rp->ai_family,
                rp->ai_family == AF_INET?
                    (const void *) ip_v4 : (const void *) ip_v6,
                iptext,
                rp->ai_family == AF_INET? INET_ADDRSTRLEN : INET6_ADDRSTRLEN
        )) == NULL){
            fprintf(stderr, "Error en la conversión de binario a textual con "
                    "inet_ntop\n");
            break;      // Cortamos la ejecución
        }

        // No se han encontrado errores
        printf("Dirección IPv%d: %s\n", rp->ai_family == AF_INET? 4:6, iptext);

        free(iptext);    // Liberamos el puntero para poder reutilizarlo
        iptext = NULL;
    }

    printf("*********************************************************"
            "*******\n");
    printf("Servicio %s: puerto %" PRId16 "\n", service, ntohs(port_or));
    /*
     * El puerto está en orden de red (big-endian). Para imprimirlo al usuario,
     * utilizamos ntohs para convertirlo a orden de host (puede ser big-endian
     * o little endian dependiendo del sistema).
     *
     * El formato de impresión definido para uint16_t es PRId16.
     */


    freeaddrinfo(res);    // Liberamos la lista enlazada
    // rp, que apuntaba a una estructura de la lista (o a NULL, si la
    // recorrió hasta el final), también queda liberado
    // Análogo para las direcciones y sockets encapsulados

    // En caso de error, puede que iptext aún no se haya liberado
    if (iptext != NULL) free(iptext);
}


void address_port_a_hostname_service(const char * addr, const char * port){
    struct sockaddr * addr;
    struct sockaddr_in addr_v4;    // Variable temporal auxiliar
    struct sockaddr_in6 addr_v6;   // Variable temporal auxiliar
    char hostname[NI_MAXHOST]; // NI_MAXHOST es (longitud máxima de un nombre)
    char service[NI_MAXSERV];
    uint16_t port_num;
    short error;

    printf("*********************************************************"
            "*******\n");

    // Comprobamos que el puerto sea numérico y esté dentro del rango límite
    if (!es_formato_port_valido(port, &port_num)) return;

    /*
     * Llamamos a setup_socket para comprobar que la dirección tenga un formato
     * correcto y, a la vez, preparar la estructura de la dirección a convertir
     */
    if (!setup_socket(ip, &addr))
        return

    // Añadimos la información del puerto a la estructura sockaddr
    if (addr->sa_family == AF_INET){
        addr_v4 = (struct sockaddr_in *) addr;
        addr_v4->sin_port = htons(port_num);   // Pasamos a orden de red
    } else {  // Sabemos, por construcción, que sa_family es AF_INET o AF_INET6

    }

    /*
     * Llamamos a getnameinfo con los siguientes argumentos:
     *      1) addr, la dirección a convertir
     *      2) el tamaño de la estructura addr (como socklen_t, un entero de
     *         al menos 32 bits)
     *      3) la cadena donde se guardará el nombre del host
     *      4) la longitud máxima de la cadena anterior
     *      5) la cadena donde guardaremos el servicio asociado al puerto
     *      6) la longitud máxima de la cadena anterior
     *      7) no necesitamos flags, por lo que dejamos el argumento a 0
     */
    error = getnameinfo((const struct sockaddr *) addr,
            (socklen_t) sizeof(*addr), hostname, (socklen_t) sizeof(hostname),
            service, (socklen_t) sizeof(service), 0);

    if (error){    // La función devuelve un valor distinto de 0
        // Mostramos el significado del código de error con gai_strerror()
        fprintf(stderr, "Error en getnameinfo: %s\n",
                gai_strerror(error_getnameinfo));
        return;
    }

    printf("Dirección IPv%d %s: %s\n", addr->sa_family == AF_INET? 4 : 6,
            ip, hostname);
    // Sabemos que el campo sa_family de addr es AF_INET o AF_INET6 porque
    // la función getnameinfo no lo modifica (es const)

    printf("*********************************************************"
            "*******\n");
    printf("Puerto: %s: servicio %s\n", port, service);

    // Liberamos la memoria que se reservó para addr en setup_socket
    free(addr);



    // Construimos el socket
    // No podemos utilizar la estructura abstracta directamente
    // Construimos el socket con IPv4 ¿¿¿DA IGUAl??
    // Hace falta sin_family??


    // Casteamos el socket a la estructura abstracta sockaddr
    error = getnameinfo((const struct sockaddr *) &addr,
            (socklen_t) sizeof(addr), NULL, 0, service, NI_MAXSERV, 0);

    if (error){
        fprintf(stderr, "Error en la función getnameinfo de port a service\n");
        exit(EXIT_FAILURE);
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

    // Cuando hay presentes simutáneamente name y service, o addr y port,
    // llamamos a funciones especializadas, en aras de una mayor eficiencia
    if (name)
        if (service)
            name_service_a_hostinfo_port(name, service);  // Nombre y servicio
        else
            name_a_hostinfo(name);    // Solo nombre
    else if (service)
        service_a_port(service);      // Solo servicio

    if (addr)
        if (port)
            address_port_a_hostname_service(addr, port);  // IP y puerto
        else
            address_a_hostname(addr);   // Solo IP
    else if (port)
        port_a_service(port);           // Solo puerto

    printf("************************************************************"
            "****\n\n");

    // Finalizamos correctamente, con codigo de salida EXIT_SUCCESS
    return (EXIT_SUCCESS);
}
