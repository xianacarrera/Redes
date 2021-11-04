#ifndef LIBCLI_H
#define LIBCLI_H

#define N 1000

short comprobar_port(const char * port, uint16_t * port_formateado);
void cerrar_con_error(char * mensaje, int con_errno);
int crear_socket();
void asignar_direccion_puerto(int sockserv, uint16_t puerto);
void marcar_pasivo(int sockserv, int max_espera);
int atender(int sockserv, struct sockaddr_in * ipportcli, int * length_ptr);
void solicitar_conexion(int sockserv, struct sockaddr_in direccion,
        uint16_t puerto);
int enviar(int sockcon, char * mensaje);
int recibir(int sockserv, char * buffer);
int recibir_nbytes(int sockserv, char * buffer, int numbytes);
void cerrar_socket(int sockcon);

#endif // LIBCLI_H
