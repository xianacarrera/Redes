#ifndef LIB_H
#define LIB_H

#define N 1000    //???????????????????? Aquí???

void comprobar_port(const char * port, uint16_t * port_formateado);
void cerrar_con_error(char * mensaje, int con_errno);
int crear_socket();
void asignar_direccion_socket(int socket, uint16_t puerto);
ssize_t recibir(int socket, char * buffer, struct sockaddr_in * dir_remota,
        size_t numbytes);
void cerrar_socket(int sockcon);

ssize_t enviar(int socket, void * mensaje, struct sockaddr_in * dir_remota,
        size_t tam);
ssize_t recibir_floats(int socket, float * buffer,
            struct sockaddr_in * dir_remota);

#endif // LIB_H
