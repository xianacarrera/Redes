#include<stdio.h>
#include<stdlib.h>

int initv(int **vfun){
    // Versión 1 -> int initv(int *vfun)
    // Versión 2 -> doble puntero. Es un puntero a un puntero.
    int n = 5;

    // Con la versión 1 (segmentation fault):
    // vfun = (int *) malloc(n*sizeof(int));

    // Versión 2:
    // Reservo memoria para el puntero de main
    *vfun = (int *) malloc(n*sizeof(int));

    for (int i = 0; i < n; i++){
        // Versión 1:
        // vfun[i] = i;

        (*vfun)[i] = i;
        // Otra opción: *(*vfun + i) = i;
    }

    return(n);
}

int main(){
    int n;
    int *v = NULL;

    // Versión 1:
    // n = initv(v);
    // Estoy pasando v por valor
    // La función recibe el valor al que apunta v, que es NULL
    // Daría segmentation fault

    // Versión 2:
    // Si en cambio, hago
    n = initv(&v);
    // Envío la dirección de un puntero. vfun es un puntero que apunta a un
    // puntero.

    for (int i = 0; i < n; i++)
        printf("v[%d] = %d\n", i, v[i]);
}
