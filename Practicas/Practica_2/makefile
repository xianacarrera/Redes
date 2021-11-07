# Opciones de compilación, mostrando todos los warnings (-Wall)
CC = gcc -Wall

# Carpeta donde se encuentran las cabeceras
HEADER_FILES_DIR = .
# Opción de compilación para incluir los .h
INCLUDES = -I $(HEADER_FILES_DIR)

# Ficheros fuente
SRCS_1 = servidor.c lib.c
SRCS_2 = cliente.c lib.c
SRCS_3 = servidormay.c lib.c
SRCS_4 = clientemay.c lib.c

# Cabeceras
LIB_HEADERS = $(HEADER_FILES_DIR)/lib.h 

# Nombres de los ejecutables del programa del servidor y del programa del cliente: 
# nombres de los ficheros fuente sin extensión
OUTPUT_1 = servidor
OUTPUT_2 = cliente
OUTPUT_3 = servidormay
OUTPUT_4 = clientemay

# Archivos objeto respectivos (.o con un .c análogo como fichero fuente)
OBJS_1 = $(SRCS_1:.c=.o)
OBJS_2 = $(SRCS_2:.c=.o)
OBJS_3 = $(SRCS_3:.c=.o)
OBJS_4 = $(SRCS_4:.c=.o)


# Regla 1
# Creamos el ejecutable de cada programa
all: $(OUTPUT_1) $(OUTPUT_2) $(OUTPUT_3) $(OUTPUT_4)

# Regla 2
# Creamos el ejecutable del programa servidor
# $@ es el nombre del archivo que se está generando, $< es el primer prerrequisito
$(OUTPUT_1): $(OBJS_1) 
	$(CC) -o $(OUTPUT_1) $(OBJS_1)

# Regla 3
# Creamos el ejecutable del programa cliente
$(OUTPUT_2): $(OBJS_2) 
	$(CC) -o $(OUTPUT_2) $(OBJS_2)
	
# Regla 4
# Creamos el ejecutable del programa servidor de mayúsculas
$(OUTPUT_3): $(OBJS_3) 
	$(CC) -o $(OUTPUT_3) $(OBJS_3)
	
# Regla 5
# Creamos el ejecutable del programa servidor de mayúsculas
$(OUTPUT_4): $(OBJS_4) 
	$(CC) -o $(OUTPUT_4) $(OBJS_4)

# Regla 6
# Creamos el archivo .o para el servidor a partir de su correspondiente .c
# Si servidor.c cambia, solo servidor.o se recompilará. cliente.o no se ve afectado
$(*.o): %.c $(LIB_HEADERS) 
	$(CC) -c -o $@ $< $(INCLUDES)

# Regla 7
# Borra los ejecutables y ejecuta clean dentro del directorio actual
cleanall: clean 
	rm -f $(OUTPUT_1) $(OUTPUT_2) $(OUTPUT_3) $(OUTPUT_4)

# Regla 8
# Borra todos los archivos .o utilizando el wildcard * (match con cualquier carácter)
# dentro del directorio actual
clean: 
	rm -f $(OUTPUT_1) $(OUTPUT_2) $(OUTPUT_3) $(OUTPUT_4) *.o
