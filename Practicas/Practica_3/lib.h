#ifndef LIB_H
#define LIB_H

#define N 1000    //???????????????????? Aquí???

void comprobar_port(const char * port, uint16_t * port_formateado);
void cerrar_con_error(char * mensaje, int con_errno);
int crear_socket();
void asignar_direccion_socket(int socket, uint16_t puerto);
int recibir(int socket, char * mensaje, struct sockaddr_in * dir_remota);
void cerrar_socket(int sockcon);

#endif // LIB_H
