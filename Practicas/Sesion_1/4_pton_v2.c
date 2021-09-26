#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>
#include<arpa/inet.h>

// **********************************************
// PASO TEXTUAL -> BINARIO CON RESERVA DE MEMORIA
// **********************************************

/* FALLO MUY HABITUAL:
 * Es común que definamos ipbin como un puntero a estructura (struct in_addr *ipbin).
 * Aun si después eliminamos los & y "reducimos un nivel", nos dará fallo de segmentación.
 * ¿Por qué? Porque si declaro un puntero y no lo apunto a nada, no está inicializado
 * (ni siquiera apunta a NULL).
 * Cuando se realiza la conversión, la función intenta escribir el resultado en una zona
 * de memoria a la que en realidad no tiene acceso.
 *
 * Nótese que el programa compilaría, pero tendría error de ejecución. No obstante, si
 * usamos la opción -Wall, nos dará una indicación al respecto.
 * 
 * Para hacer esto, tendríamos que hacer una reserva dinámica de memoria (memory alloc):
 * ipbin = malloc(sizeof(struct in_addr));
 *
 * Nota: malloc, por defecto, devuelve un puntero a void. Por eso hacemos un cast a struct:
 * ipbin = (struct in_addr *) malloc(sizeof(struct in_addr));
 *
 * Nota2: Además, si malloc no encuentra espacio, devuelve un error. Se suele poner:
 * if ((ipbin = (struct in_addr *) malloc(sizeof(struct in_addr))) == NULL){
 *     fprintf(stderr, "Error reservando memoria");
 *     exit(EXIT_FAILURE);
 * }
 *
 * Quizás la opción de redes4v1.c sea más sencilla
 */

int main(){
    struct in_addr * ipbin;
    char iptext[INET_ADDRSTRLEN] = "193.110.128.200";
    int error;
    
    if ((ipbin = (struct in_addr *) malloc(sizeof(struct in_addr))) == NULL){
      fprintf(stderr, "Error reservando memoria\n");
      exit(EXIT_FAILURE);
    }
    
    /* Imp: 
     * - Acordarse de poner & en el tercer parametro.
     * - Guardar la referencia de si el resultado ha sido erróneo o no para
     *   gestionarlo (siempre tenemos que usar salvaguardas en funciones de red)
     * - Aunque es opcional, podemos castear el 2º y el 3º campo para que coincidan
     *   exactamente con los tipos de datos definidos por la función inet_pton
     */
    error = inet_pton(AF_INET, (const char *) iptext, (void *) ipbin);
    
    if (error != 1){
        /* 
         * fprint es una variante de printf que permite escribir en fichero.
         * Al pasarle stderr, escribe ese mensaje por pantalla como salida de error.
         * La diferencia está en que el usuario puede configurar la salida de error
         * (por ejemplo, que escriba en un fichero determinado).
         */
        fprintf(stderr, "Error en inet_pton\n");  
        
        // La función solo puede devolver 1, 0 o -1
        if (error == 0)
            fprintf(stderr, "Dirección %s incorrecta\n", iptext);
        else if (error == -1)
            fprintf(stderr, "Campo AF incorrecto\n");
        else     // Esta condición nunca debería darse
            fprintf(stderr, "Error indefinido\n");
        
        exit(EXIT_FAILURE);    // Cierro programa con estado de error
    }
        
    printf("IPBIN = 0x%X\n", ipbin->s_addr);
    
     // Es más elegante poner que main siempre devuelva un entero 
     // (aunque se hace automáticamente)
    return(EXIT_SUCCESS);   
}
