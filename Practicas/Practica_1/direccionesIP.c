#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<ctype.h>
#include<string.h>
#include<limits.h>
#include<errno.h>


/*
 * Autora: Xiana Carrera Alonso
 * Redes - Curso 2020/2021
 *
 * Práctica 1 - Manejo de direcciones IP en C
 *
 * Este programa proporciona la información asociada a un nombre de host/nombre
 * de servicio/dirección IP/número de puerto introducido por el usuario.
 */

// Funciones principales
void name_a_hostinfo(const char * name);
void service_a_port(const char * service);
void address_a_hostname(const char * ip);
void port_a_service(const char * port);
void name_service_a_hostinfo_port(const char * name, const char * service);
void address_port_a_hostname_service(const char * ip, const char * port);

// Funciones auxiliares
short comprobar_IP(const char * ip, struct in_addr * ipv4bin,
    struct in6_addr * ipv6bin);
short setup_socket(const char * ip, struct sockaddr **addr);
short comprobar_port(const char * port, uint16_t * port_formateado);



int main(int argc, char * argv[]) {
    int opt;
    // name: Nombre del host
    // service: Nombre del servicio
    // addr: Direccion IPv4 o IPv6
    // port: Puerto
    char * name = NULL, * service = NULL, * addr = NULL, * port = NULL;

    // Comprobamos que exista al menos un operando
    // En caso contrario terminamos la ejecución con código EXIT_FAILURE
    if (argc < 2) {
        printf("Falta un operando\n");
        printf("Usar: %s [-n Nombre del host] [-s Nombre del servicio "
                "(p.e. http)] [-i Direccion ip] [-p Numero de puerto]\n",
                argv[0]);
        return (EXIT_FAILURE);
    }

    // Utilizamos getopt() para manejar los operandos de la línea de comandos
    // Las opciones n: s: i: p: indican que esos "flags" (nsip) tienen que
    // introducirse seguidos de un argumento
    // El argumento leído se guarda en la variable externa optarg
    while ((opt = getopt(argc, argv, ":n:s:i:p:")) != -1) {
        switch (opt) {
            case 'n':
                name = optarg;     // Argumento nombre de host
                break;
            case 's':
                service = optarg;  // Argumento nombre de servicio
                break;
            case 'i':
                addr = optarg;    // Argumento direccion ip
                break;
            case 'p':
                port = optarg;    // Argumento numero de puerto
                break;
            case ':':    // Se intrudujo un flag sin argumento obligatorio
                fprintf(stderr, "La opción -%c requiere un argumento.\n",
                        optopt);
                return (EXIT_FAILURE);    // Cortamos la ejecución
            case '?':
                if (isprint(optopt))  // Se introdujo un flag no permitido
                    fprintf(stderr, "Opción desconocida `-%c'.\n", optopt);
                else      // Hay un carácter no legible en las opciones
                    fprintf(stderr, "Caracter de opción desconocido"
                            " `\\x%x'.\n", optopt);
                return (EXIT_FAILURE);   // Cortamos la ejecución
            default: // Se produjo un error indeterminado. Nunca se deberia llegar aqui.
                abort();    // Finalización anormal del programa
            }
    }

    printf("\n");
    // Llamamos a las funciones correspondientes a los argumentos solicitados
    // (que hemos verificado que son != NULL)

    // Cuando hay presentes simutáneamente name y service, o addr y port,
    // llamamos a funciones especializadas, en aras de una mayor eficiencia
    if (name)
        if (service)
            name_service_a_hostinfo_port(name, service);  // Nombre y servicio
        else
            name_a_hostinfo(name);    // Solo nombre
    else if (service)
        service_a_port(service);      // Solo servicio

    // Análogo para los casos combinados de dirección IP y puerto
    if (addr)
        if (port)
            address_port_a_hostname_service(addr, port);  // IP y puerto
        else
            address_a_hostname(addr);   // Solo IP
    else if (port)
        port_a_service(port);           // Solo puerto

    printf("************************************************************"
            "****\n\n");

    // Finalizamos correctamente, con código de salida EXIT_SUCCESS
    return (EXIT_SUCCESS);
}


/*
 * Función que a partir del nombre de un host, determina su nombre canónico
 * y todas las IPs que tiene asociado, sean IPv4 o IPv6.
 *
 * La función no devuelve nada. En caso de error, imprime un mensaje con una
 * descripción del mismo y termina la ejecución del programa.
 * Si no tienen lugar fallos, imprime los resultados obtenidos.
 *
 * Parámetros:
 * const char * name -> (Entrada) cadena con el nombre del host a localizar.
 *
 */
void name_a_hostinfo(const char * name){
    struct addrinfo *res;     // Lista enlazada que guardará el resultado
    struct addrinfo *rp;      // Puntero para avanzar por el resultado
    struct addrinfo hints;    // Configuración de opciones de getaddrinfo
    struct sockaddr_in * socka_v4;   // Representa socket con dirección IPv4
    struct sockaddr_in6 * socka_v6;  // Representa socket con dirección IPv6
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
        fprintf(stderr, "Error de getaddrinfo en name_a_hostinfo: %s\n\n",
                gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    // Accedemos al nombre canónico a través de la primera estructura
    printf("Nombre canónico: %s\n", res->ai_canonname);

    // Recorremos la lista enlazada de estructuras
    for (rp = res; rp != NULL; rp = rp->ai_next){
        if (rp->ai_family == AF_INET){              // Dirección IPv4
            socka_v4 = (struct sockaddr_in *) rp->ai_addr;
            ip_v4 = &(socka_v4->sin_addr);          // Guardamos la dirección

            // Reservamos espacio para la dirección textual
            // La longitud máxima será INET_ADDRSTRLEN al ser una IPv4
            if ((iptext = (char *) malloc(INET_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria en "
                        "name_a_hostinfo\n\n");
                exit(EXIT_FAILURE);
            }

        } else if (rp->ai_family == AF_INET6){     // Dirección IPv6
            socka_v6 = (struct sockaddr_in6 *) rp->ai_addr;
            ip_v6 = &(socka_v6->sin6_addr);        // Guardamos la dirección

            // Reservamos espacio para la dirección textual
            // La longitud máxima será INET6_ADDRSTRLEN al ser una IPv6
            if ((iptext = (char *) malloc(INET6_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria en "
                        "name_a_hostinfo\n\n");
                exit(EXIT_FAILURE);
            }

        } else {      // No se debería dar nunca
            fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido "
                    "con getaddrinfo en name_a_hostinfo\n\n");
            // Dado que desconocemos si el resto de resultados son correctos,
            // cortamos la ejecución
            exit(EXIT_FAILURE);
        }

        /*
        * Convertimos la dirección obtenida de binario a textual
        * Pasamos como argumentos:
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
            // inet_ntop actualiza errno de acuerdo al error ocurrido
            fprintf(stderr, "Error en la conversión de binario a textual con "
                    "inet_ntop en name_a_hostinfo: %s\n\n", strerror(errno));
            exit(EXIT_FAILURE);      // Cortamos la ejecución
        }

        // No se han encontrado errores
        printf("\tDirección IPv%d: %s\n",
                rp->ai_family == AF_INET? 4 : 6, iptext);

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


/*
 * Función que a partir del nombre de un servicio, determina cuál es su número
 * de puerto asociado.
 *
 * La función no devuelve nada. En caso de error, imprime un mensaje con una
 * descripción del mismo y termina la ejecución del programa. Si no tienen
 * lugar fallos, imprime sus resultados, esto es, el número de puerto
 * asociado al servicio.
 *
 * Parámetros:
 * const char * service -> (entrada) cadena con el nombre del servicio a
 * traducir. (La salida de la función también es correcta si se pasa el número
 * de puerto del servicio en lugar de su nombre. Pero, en tal caso, el
 * resultado es el propio número introducido como entrada, por lo que no
 * resulta útil).
 *
 */
void service_a_port(const char * service){
    struct addrinfo *res;          // Guardará el resultado de getaddrinfo
    struct addrinfo hints;         // Configuración de opciones de getaddrinfo
    uint16_t port_or;              // Puerto obtenido en orden de red
    int error;                     // Gestión de errores de getaddrinfo

    printf("*********************************************************"
            "*******\n");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;  // Nos es indiferente seleccionar IPv4 o IPv6
    /*
     * ai_family se indica por coherencia, pero en realidad, al dejar el
     * nombre del host a NULL en getaddrinfo, la dirección del resultado
     * será la del propio sistema (loopback): INADDR_LOOPBACK para IPv4
     * o IN6ADDR_LOOPBACK_INIT para IPv6
     *
     * Los campos ai_socktype, ai_protocol y ai_flags se mantienen a 0,
     * indicando que se acepta cualquier tipo de socket, cualquier protocolo,
     * y sin seleccionar opciones adicionales, respectivamente.
     */

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
        fprintf(stderr, "Error de getaddrinfo en service_a_port: %s\n",
                gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    // El resultado contendrá una única estructura addrinfo
    if (res->ai_family == AF_INET){
        port_or = ((struct sockaddr_in *) res->ai_addr)->sin_port;
    } else if (res->ai_family == AF_INET6){
        port_or = ((struct sockaddr_in6 *) res->ai_addr)->sin6_port;
    } else {
        fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido con "
                "getaddrinfo en service_a_port\n\n");
        exit(EXIT_FAILURE);
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


/*
 * Función que a partir de una dirección IP determina el nombre de su host
 * asociado. Se aceptan tanto direcciones IPv4 como IPv6.
 *
 * La función no devuelve nada. En caso de error, imprime un mensaje con una
 * descripción del mismo. Si la ejecución transcurre sin fallos, imprime sus
 * resultados, esto es, el nombre del host correspondiente a la IP.
 *
 * Parámetros:
 * const char * ip -> (entrada) cadena con el nombre de la IP a buscar.
 *
 */
void address_a_hostname(const char * ip){
    struct sockaddr * addr;    // Representa un socket
    char hostname[NI_MAXHOST]; // Cadena que guardará el nombre del host
        // NI_MAXHOST indica la longitud máxima que puede tener un nombre
    int error;                 // Gestión de errores

    printf("*********************************************************"
            "*******\n");

    /*
     * Llamamos a setup_socket para comprobar que el input tenga el formato
     * correcto y, a la vez, preparar la estructura del socket, encapsulando
     * la dirección a convertir.
     */
    if (!setup_socket(ip, &addr))      // Si devuelve 0, hubo un error
        exit(EXIT_FAILURE);

    /*
     * Llamamos a getnameinfo con los siguientes argumentos:
     *      1) addr, que contiene la dirección a convertir
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
            addr->sa_family == AF_INET? (socklen_t) sizeof(struct sockaddr_in)
                    : (socklen_t) sizeof(struct sockaddr_in6),
            hostname, (socklen_t) sizeof(hostname), NULL, 0, 0);

    if (error){    // La función devuelve un valor distinto de 0
        // Mostramos el significado del código de error con gai_strerror()
        fprintf(stderr, "Error de getnameinfo en address_a_hostname: %s\n\n",
                gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    // Si no hubo errores, imprimimos el resultado
    printf("Dirección IPv%d %s: host %s\n", addr->sa_family == AF_INET? 4 : 6,
            ip, hostname);
    // Sabemos que el campo sa_family de addr es AF_INET o AF_INET6 porque
    // la función getnameinfo mantiene constante lo contruido en setup_socket

    // Liberamos la memoria que se reservó para addr en setup_socket
    free(addr);
}


/*
 * Función que obtiene el nombre de servicio asociado a un determinado puerto.
 * No devuelve nada. En caso de error, imprime un mensaje descriptivo.
 * Si la ejecución transcurre sin problemas, imprime el resultado obtenido.
 *
 * Parámetros:
 * const char * port -> (entrada) Cadena con el puerto asociado al servicio.
 *
 */
void port_a_service(const char * port){
    struct sockaddr_in addr;   // Guardará el puerto. Entrada de getnameinfo
    uint16_t port_num;         // Puerto en formato entero
    char service[NI_MAXSERV];  // Cadena que guardará el nombre del servicio
        // NI_MAXSERV es la longitud máxima de un nombre de servicio
    int error;                 // Gestión de errores de getnameinfo

    printf("*********************************************************"
            "*******\n");

    // Comprobamos que el input sea numérico y esté dentro del rango límite
    // En caso afirmativo, se guardará el entero correspondiente en port_num
    if (!comprobar_port(port, &port_num)) exit(EXIT_FAILURE);

    /*
     * Indicamos la información necesaria en la estructura de socket
     * No podemos utilizar la estructura abstracta directamente, de modo que
     * tomamos un sockaddr_in (IPv4). Un sockaddr_in6 también sería válido.
     * Debemos pasar el puerto de orden de host a orden de red con htons.
     */
    addr.sin_port = htons(port_num);
    addr.sin_family = AF_INET;          // IPv4

    /*
     * Llamamos a getnameinfo con argumentos:
     *      1) addr casteado a un puntero a la estructura abstracta sockaddr
     *      2) el tamaño de la estructura addr (como socklen_t, un entero de
     *         al menos 32 bits)
     *      3) NULL, ya que no vamos a recibir el nombre de ningún host
     *      4) 0, puesto que el anterior argumento es nulo
     *      5) la cadena en la que guardaremos el nombre del servicio
     *      6) la longitud máxima que puede tener esa cadena
     *      7) no necesitamos flags, por lo que dejamos el argumento a 0
     */
    error = getnameinfo((const struct sockaddr *) &addr,
            (socklen_t) sizeof(addr), NULL, 0,
            service, (socklen_t) NI_MAXSERV, 0);

    if (error){
        // gai_strerror describe el error en función del código recibido
        fprintf(stderr, "Error de getaddrinfo en port_a_service: %s\n\n",
                gai_strerror(error));
        return;
    }

    // Como no hay error, imprimimos el resultado obtenido
    printf("Puerto: %s: servicio %s\n", port, service);
}


/*
 * Función que, en aras de una mayor eficiencia, combina las funcionalidades
 * de name_a_hostinfo y service_a_port, dando una resolución específica
 * al caso en el que el usuario introduce tanto nombre como servicio.
 *
 * Su propósito es dar información (nombre canónico y direcciones IP) de un
 * host a partir de su nombre, y convertir el nombre de un servicio en su
 * número de puerto asociado.
 *
 * En caso de algún tipo de error, se imprime un mensaje descriptivo y se
 * corta la ejecución del programa. Si la ejecución es correcta, se imprime
 * el resultado. No se devuelve nada.
 *
 * Parámetros:
 * const char * name -> (Entrada) cadena con el nombre del host a localizar.
 * const char * service -> (Entrada) cadena con el nombre del servicio a
 *      traducir. Si se introduce un número de puerto en lugar de un nombre,
 *      el resultado será el propio número.
 *
 */
void name_service_a_hostinfo_port(const char * name, const char * service){
    struct addrinfo *res;     // Lista enlazada que guardará el resultado
    struct addrinfo *rp;      // Puntero para avanzar por el resultado
    struct addrinfo hints;    // Configuración de opciones de getaddrinfo
    struct sockaddr_in * socka_v4;   // Representa socket con dirección IPv4
    struct sockaddr_in6 * socka_v6;  // Representa socket con dirección IPv6
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


    /*
     * Llamamos a getaddrinfo indicando el nombre a convertir.
     * Indicamos además el servicio dado por el usuario como 2º argumento.
     * Pasamos las opciones seleccionadas como puntero a hints.
     * El resultado se guardará en res.
     */
    error = getaddrinfo(name, service, (const struct addrinfo *) &hints, &res);

    if (error){
        // gai_strerror describe el error en función del código recibido
        fprintf(stderr, "Error de getaddrinfo en name_service_a_hostinfo_port:"
                " %s\n\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    // Podemos acceder al nombre canónico y al puerto desde la 1ª estructura
    // (El puerto se obtiene con el servicio, no depende de la estructura)
    printf("Nombre canónico: %s\n", res->ai_canonname);
    if (res->ai_family == AF_INET){           // Puerto configurado para IPv4
        port_or = ((struct sockaddr_in *) res->ai_addr)->sin_port;
    } else if (res->ai_family == AF_INET6){   // Puerto configurado para IPv6
        port_or = ((struct sockaddr_in6 *) res->ai_addr)->sin6_port;
    } else {
        fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido con "
                "getaddrinfo en name_service_a_hostinfo_port\n\n");
        exit(EXIT_FAILURE);
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
                fprintf(stderr, "Error reservando memoria en "
                        "name_service_a_hostinfo_port\n\n");
                exit(EXIT_FAILURE);
            }

        } else if (rp->ai_family == AF_INET6){     // Dirección IPv6
            socka_v6 = (struct sockaddr_in6 *) rp->ai_addr;
            ip_v6 = &(socka_v6->sin6_addr);        // Guardamos la dirección

            // Reservamos espacio para la dirección en formato textual
            // La longitud máxima será INET6_ADDRSTRLEN (es una IP_v6)
            if ((iptext = (char *) malloc(INET6_ADDRSTRLEN*sizeof(char)))
                    == NULL){
                fprintf(stderr, "Error reservando memoria en "
                                "name_service_a_hostinfo_port\n\n");
                exit(EXIT_FAILURE);
            }

        } else {      // Nunca se debería dar
            fprintf(stderr, "Se ha obtenido un tipo de resultado desconocido "
                    "con getaddrinfo en name_service_a_hostinfo_port\n\n");
            // Paramos la ejecución al no saber si el resto de resultados
            // serán correctos o no
            exit(EXIT_FAILURE);
        }

        /*
        * Convertimos la dirección obtenida de binario a textual
        * Pasamos como argumentos:
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
                    "inet_ntop en name_service_a_hostinfo_port: %s\n\n",
                    strerror(errno));
            exit(EXIT_FAILURE);      // Cortamos la ejecución
        }

        // No se han encontrado errores
        printf("\tDirección IPv%d: %s\n",
                rp->ai_family == AF_INET? 4 : 6, iptext);

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


/*
 * Función que, en aras de una mayor eficiencia, combina las funcionalidades
 * de address_a_hostname y port_a_service, dando una resolución específica
 * al caso en el que el usuario introduce tanto dirección como puerto.
 *
 * Su propósito es buscar y mostrar el nombre del host asociado a la
 * dirección introducida, sea esta IPv4 o IPv6, e indicar el nombre del
 * servicio asociado al puerto pasado como argumento.
 *
 * En caso de algún tipo de error, se imprime un mensaje descriptivo y se
 * corta la ejecución del programa. Si la ejecución es correcta, se imprime
 * el resultado. No se devuelve nada.
 *
 * Parámetros:
 * const char * ip -> (entrada) cadena con el nombre de la IP a buscar.
 * const char * port -> (entrada) Cadena con el puerto asociado al servicio.
 */
void address_port_a_hostname_service(const char * ip, const char * port){
    struct sockaddr * addr;          // Representa un socket
    struct sockaddr_in * addr_v4;    // Variable temporal auxiliar
    struct sockaddr_in6 * addr_v6;   // Variable temporal auxiliar
    char hostname[NI_MAXHOST]; // Cadena que guardará el nombre del host
        // NI_MAXHOST indica la longitud máxima que puede tener un nombre
    char service[NI_MAXSERV];  // Cadena que guardará el nombre del servicio
        // NI_MAXSERV es la longitud máxima de un nombre de servicio
    uint16_t port_num;         // Puerto en formato entero
    int error;                 // Gestión de errores de getnameinfo

    printf("*********************************************************"
            "*******\n");

    // Comprobamos que el puerto sea numérico y esté dentro del rango límite
    if (!comprobar_port(port, &port_num)) exit(EXIT_FAILURE);

    /*
     * Llamamos a setup_socket para comprobar que la dirección tenga un formato
     * correcto y, a la vez, preparar la estructura de la dirección a convertir
     */
    if (!setup_socket(ip, &addr)) exit(EXIT_FAILURE);

    // Añadimos la información del puerto a la estructura sockaddr
    if (addr->sa_family == AF_INET){
        addr_v4 = (struct sockaddr_in *) addr;
        addr_v4->sin_port = htons(port_num);    // Pasamos a orden de red
        addr = (struct sockaddr *) addr_v4;     // Cast permitido
    } else {  // Sabemos, por construcción, que sa_family es AF_INET o AF_INET6
        addr_v6 = (struct sockaddr_in6 *) addr;
        addr_v6->sin6_port = htons(port_num);  // Pasamos a orden de red
        addr = (struct sockaddr *) addr_v6;    // Cast permitido
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
            addr->sa_family == AF_INET? (socklen_t) sizeof(struct sockaddr_in)
                    : (socklen_t) sizeof(struct sockaddr_in6),
            hostname, (socklen_t) sizeof(hostname),
            service, (socklen_t) sizeof(service), 0);

    if (error){    // La función devuelve un valor distinto de 0
        // Mostramos el significado del código de error con gai_strerror()
        fprintf(stderr, "Error con getnameinfo en "
                "address_port_a_hostname_service: %s\n\n",
                gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    printf("Dirección IPv%d %s: host %s\n", addr->sa_family == AF_INET? 4 : 6,
            ip, hostname);
    // Sabemos que el campo sa_family de addr es AF_INET o AF_INET6 porque
    // la función getnameinfo no lo modifica (es const)

    printf("*********************************************************"
            "*******\n");
    printf("Puerto: %s: servicio %s\n", port, service);

    // Liberamos la memoria que se reservó para addr en setup_socket
    free(addr);
}


/*
 * Función auxiliar que comprueba si una determinada cadena presenta un
 * formato de dirección IP válido.
 * En caso afirmativo, la encapsula en una estructura in_addr o in6_addr,
 * según sea IPv4 o IPv6, respectivamente.
 * En caso contrario, describe el error que ha tenido lugar. No se finaliza
 * aquí la ejecución (se lega la decisión a las funciones principales).
 *
 * Parámetros:
 * const char * ip -> (Entrada) cadena con la dirección a verificar
 * struct in_addr * ipv4bin -> (Salida) estructura donde se guardará la IP
 *         encapsulada, en caso de que se trate de una IPv4
 * struct in6_addr * ipv6bin -> (Salida) estructura donde se guardará la IP
 *         encapsulada, en caso de que se trate de una IPv6
 *
 * La función devuelve:
 *      2 -> Si ip es una dirección IPv4 correcta
 *      1 -> Si ip es una dirección IPv6 correcta
 *      0 -> En caso de error
 *
 */
short comprobar_IP(const char * ip, struct in_addr * ipv4bin,
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
                    "ni formato IPv6 válidos\n\n");
            return 0;
        case -1:
            fprintf(stderr, "La familia de direcciones pasada como argumento "
                    "(AF) a inet_pton en comprobar_IP es incorrecta\n\n");
            return 0;
        default:
            fprintf(stderr, "Error desconocido de inet_pton en "
                    "comprobar_IP\n\n");
    }

    return 0;     // Resultado de error
}


/*
 * Función auxiliar que prepara una estructura representativa de un socket
 * para una dirección IP. Según tenga formato IPv4 o IPv6, se guardará en un
 * sockaddr_in o un sockaddr_in6, pero el resultado se devuelve de forma
 * genérica como un sockaddr a través de un parámetro.
 *
 * La función también comprueba que la ip sea correcta llamando a comprobar_IP.
 *
 * En caso de error, se imprime un mensaje descriptivo. No se finaliza aquí
 * la ejecución (se deja la decisión a manos de las funciones principales).
 *
 * Parámetros:
 * const char * ip -> (Entrada) cadena con la dirección a encapsular.
 * struct sockaddr **addr -> (Salida) puntero a puntero de una estructura
 *         sockaddr que será apuntado a la representación adecuada del socket.
 *
 * La función devuelve:
 *      1 -> Ejecución correcta
 *      0 -> En caso de error
 *
 */
short setup_socket(const char * ip, struct sockaddr **addr){
    struct in_addr ipv4bin;         // Encapsula una dirección IPv4 binaria
    struct in6_addr ipv6bin;        // Encapsula una dirección IPv6 binaria
    struct sockaddr_in * addr_v4;   // Representa socket con dirección IPv4
    struct sockaddr_in6 * addr_v6;  // Representa socket con dirección IPv6

    /*
     * Llamamos a comprobar_IP para comprobar si la IP es válida.
     * Si es así, nos la guarda en formato binario en ipv4bin o ipv6bin,
     * según corresponda.
     */
    switch (comprobar_IP(ip, &ipv4bin, &ipv6bin)) {
        case 2:        // Dirección IPv4 válida
            // Reservamos memoria para el sockaddr_in
            if ((addr_v4 = (struct sockaddr_in *)
                        malloc(sizeof(struct sockaddr_in))
                ) == NULL){
                fprintf(stderr,"Error reservando memoria en setup_socket\n\n");
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
                fprintf(stderr,"Error reservando memoria en setup_socket\n\n");
                return 0;     // Error
            }

            addr_v6->sin6_family = AF_INET6;   // Es una IPv6
            addr_v6->sin6_addr = ipv6bin;     // Asignamos directamente la IP
            // El campo sin6_flowinfo no se usa
            // El campo sin6_port nos es indiferente al no conocer el puerto

            // El cast de sockaddr_in6 a sockaddr está soportado
            *addr = (struct sockaddr *) addr_v6;    // Damos la referencia
            return 1;      // Éxito

        case 0:
            // Hubo un error en comprobar_IP (la propia función lo imprime)
            return 0;
        default:        // Nunca debería darse
            fprintf(stderr, "Error no documentado en comprobar_IP. "
                    "Recibido en setup_socket\n\n");
    }
    return 0;
}

/*
 * Función que comprueba si una determinada cadena de caracteres tiene un
 * formato de puerto válido, esto es, que es un entero en el rango
 * [0, 65535].
 *
 * Si en efecto, se trata de un puerto correcto, la función devuelve 1 y
 * guarda en port_formateado el entero correspondiente.
 * En caso contrario, se imprime una descripción del problema y se devuelve 0.
 * La ejecución no se termina aquí, sino en las funciones principales.
 *
 * Parámetros:
 * const char * port -> (entrada) cadena con el puerto a convertir
 * uint16_t * port_formateado -> (salida) dirección del uint16_t donde se
 *      guardará el puerto en formato entero. uint16_t es un entero sin signo
 *      de 16 bits, el tipo definido para puertos (cabecera inttypes.h).
 *
 * La función devuelve:
 *      0 -> En caso de error
 *      1 -> Si la conversión se realizó correctamente
 */
short comprobar_port(const char * port, uint16_t * port_formateado){
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
        fprintf(stderr, "El puerto no es válido\n\n");
        return 0;
    }

    // Comprobamos que no haya tenido lugar un overflow
    // ULONG_MAX es una macro definida en limits.h
    if (port_numerico == ULONG_MAX && errno == ERANGE){
        fprintf(stderr, "El puerto está fuera del rango "
                "representable\n");
        perror("Error del sistema: ");   // Imprimos el error que da errno
        fprintf(stderr, "\n");
        return 0;
    }

    if (port_numerico > 65535){
        fprintf(stderr, "El puerto está fuera del rango "
                "válido para una IP: [0, 65535]\n\n");
        return 0;
    }

    // El input del usuario es correcto
    *port_formateado = (uint16_t) port_numerico;
    return 1;
}
