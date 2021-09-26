#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>

int main(){
    in_addr_t ipbin = 0xC8806EC1;
    
    /* Para evitar la incomodidad que supone el hexadecimal, podemos usar un array de char:
     * char iptext[16] = "193.110.128.200";
     * Por si se nos olvida que son 16, podemos usar una macro definida en nettinet
     */
    char iptext[INET_ADDRSTRLEN] = "193.110.128.200";  // Misma direccion que la anterior
    
    // Muchas funciones de C requieren que la direccion se pase como una estructura:
    struct in_addr ipbin2;

    
    // Imprimimos en hexadecimal
    printf("IPBIN = 0x%X\n", ipbin);
    
    // Imprimimos el array
    printf("IPTEXT = %s\n", iptext);
    
    // El unico campo de la estructura es s_addr
    ipbin2.s_addr = 0xC8806EC1;
    printf("IPBIN2 = 0x%X\n", ipbin2.s_addr);
}
