#ifndef LIB_H
#define LIB_H

/*
 * Xiana Carrera Alonso
 * Redes - Práctica 2
 * Curso 2021/2022
 *
 * Esta librería recoge funciones de utilidad para los programas servidor y
 * cliente utilizados en la practica 2.
 */

#define N 1000    // Tamaño máximo de una línea de archivo y de los mensajes

/**
 * Función que comprueba la validez de un puerto y lo convierte a uint16_t
 * @param port Puerto a validar
 * @param port_formateado Puerto pasado a formato entero
 */
void comprobar_port(const char * port, uint16_t * port_formateado);

/*
 * Función que imprime un mensaje de error (con o sin la descripción de errno)
 * y finaliza la ejecución
 * @param mensaje Mensaje a imprimir
 * @param con_errno 0 para no mostrar la descripción de errno, !0 para verla
 */
void cerrar_con_error(char * mensaje, int con_errno);

/*
 * Función que crea un socket IPv4 y orientado a conexión y devuelve su
 * identificador
 */
int crear_socket();

/*
 * Función que asigna la IP INADDR_ANY y un puerto a un socket
 * @param sockserv Identificador del socket sobre el que se hará la asignación
 * @param puerto Puerto que se ligará al socket
 */
void asignar_direccion_puerto(int sockserv, uint16_t puerto);

/*
 * Función que pone un socket a la escucha de conexiones de clientes
 * @param sockserv Identificador del socket que se marcará como pasivo
 * @param max_espera Número máximo de clientes que podrá tener esperando
 */
void marcar_pasivo(int sockserv, unsigned int max_espera);

/*
 * Función que acepta la conexión de un cliente y devuelve el identificador
 * del socket de conexión.
 * @param sockserv Identificador del socket del servidor
 * @param ipportcli Estructura que guardará la dirección del cliente
 */
int atender(int sockserv, struct sockaddr_in * ipportcli);

/*
 * Función que solicita la conexión con un servidor.
 * @param sockcli Identificador del socket sobre el que se establecerá
                  la conexión
 * @param direccion Estructura con la dirección (IP + puerto) del servidor
 */
void solicitar_conexion(int sockcli, struct sockaddr_in direccion);

/*
 * Función que envía un mensaje a través de una conexión y devuelve el número
 * de bytes transmitidos.
 * @param sockcon Identificador del socket de la conexión
 * @param mensaje Mensaje a enviar
 */
ssize_t enviar(int sockcon, char * mensaje);

/*
 * Función que envía un cierto número de bytes de un mensaje a través de una
 * conexión y devuelve el número de bytes transmitidos.
 * @param sockcon Identificador del socket de la conexión
 * @param mensaje Mensaje a enviar
 * @param numbytes Número de bytes del mensaje a enviar
 */
ssize_t enviar_nbytes(int sockcon, char * mensaje, size_t numbytes);

/*
 * Función que permite recoger datos enviados a través de una conexión.
 * @param sockcon Identificador del socket de la conexión
 * @param buffer Puntero donde se guardarán los datos
 */
ssize_t recibir(int sockcon, char * buffer);

/*
 * Función que permite recoger un cierto número de bytes de los datos enviados
 * a través de una conexión.
 * @param sockcon Identificador del socket de la conexión
 * @param buffer Puntero donde se guardarán los datos
 * @param numbytes Número de bytes a recibir
 */
ssize_t recibir_nbytes(int sockcon, char * buffer, size_t numbytes);

/*
 * Función que cierra un socket
 * @param sock Identificador del socket a cerrar
 */
void cerrar_socket(int sock);


#endif // LIB_H
