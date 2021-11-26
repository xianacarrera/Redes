Xiana Carrera Alonso
Redes - Práctica 2
Curso 2021/2022

                                Compilación
                                
Se ha incluido un makefile que genera los ejecutables de los 4 programas (servidor.c, cliente.c,
servidormay.c y clientemay.c) como opción por defecto, "make", enlazando además la librería 
lib.c, que todos ellos usan. Se incluye también una opción para eliminar los .o residuales,
utilizando "make clean". Para eliminar tanto los .o como los ejecutables, se puede usar "make
cleanall".

                                Apartados 1c), 1d), 3
                                
El código por defecto para servidor.c y cliente.c corresponde a los apartados 1a) y 1b),
respectivamente. En ambos archivos se indica qué líneas se deben tapar y destapar para
comprobar el funcionamiento de los apartados 1c) y 1d).

La espera de clientemay.c indicada en el apartado 3 sí aparece explícita y destapada
por defecto. No es necesario ningún cambio para comprobar dicho apartado.


                                Cierre de los servidores
                                
Los servidores se cierran automáticamente al cabo de 30 segundos de inactividad (sin recibir 
datos). Este valor es modificable desde servidor.c y servidormay.c.


