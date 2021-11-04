#ifndef LIBSERV_H
#define LIBSERV_H

short comprobar_port(const char * port, uint16_t * port_formateado);
void cerrar_con_error(char * mensaje, int con_errno);
int crear_socket();
void asignar_direccion_puerto(int sockserv, uint16_t puerto);
void marcar_pasivo(int sockserv, int max_espera);
int atender(int sockserv, struct sockaddr_in * ipportcli, int * length_ptr);
int enviar(int sockcon, char * mensaje);
int recibir(int sockserv, char * buffer);
void cerrar_socket(int sockcon);

#endif // LIBSERV_H
