#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>

// Para ejecutar:
// gcc nombreFichero.c
// ./a.out

int main(){
    // ¿Como declaro una variable de exactamente 32 bits?
    
    // Las siguientes no me sirven:
    unsigned int ui;
    unsigned short int usi;
    unsigned long int uli;
    unsigned long long int ulli;
    
    // Con estos tipos de datos sí podemos fijar el tamaño:
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
    uint64_t u64;
    
    // También existe int32_t, que tiene signo
    
    // No obstante, el tipo de dato que siempre vamos a usar para direcciones IP será:
    in_addr_t ipbin;
    
    
    // %ld porque la salida de sizeof es un entero largo
    printf("USI = %ld bytes\n", sizeof(usi));      // 2 bytes
    printf("UI = %ld bytes\n", sizeof(ui));        // 4 bytes
    printf("ULI = %ld bytes\n", sizeof(uli));      // 8 bytes
    printf("ULLI = %ld bytes\n", sizeof(ulli));    // 8 bytes
    printf("\n\n");
    
    // El lenguaje C no indica cuánto tiene que ocupar un entero en memoria
    // Depende del dispositivo particular
    
    printf("U8 = %ld bytes\n", sizeof(u8));      // 1 byte
    printf("U16 = %ld bytes\n", sizeof(u16));    // 2 bytes
    printf("U32 = %ld bytes\n", sizeof(u32));    // 4 bytes
    printf("U64 = %ld bytes\n", sizeof(u64));    // 8 bytes
    printf("\n\n");
    
    printf("IPBIN = %ld bytes\n", sizeof(ipbin));
}





