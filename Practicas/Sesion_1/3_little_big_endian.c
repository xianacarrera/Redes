#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>

// ******************************
// LITTLE ENDIAN Y BIG ENDIAN
// ******************************


int main(){
    struct in_addr ipbin;
    uint8_t *ipbyte;
    char iptext[INET_ADDRSTRLEN] = "193.110.128.200";
    
    ipbin.s_addr = 0xC8806EC1;

    // ipbyte = &(ipbin.s_addr);
    // Me da un warning; debo castear
    
    ipbyte = (uint8_t *) &(ipbin.s_addr);  
    // ipbyte apunta a la primer direccion de memoria de la direccion IP
    
    printf("IPBIN = 0x%X\n", ipbin.s_addr);
    printf("IPTEXT = %s\n", iptext);
    printf("IPBYTE[0] = %d = 0x%X\n", ipbyte[0], ipbyte[0]);
    printf("IPBYTE[1] = %d = 0x%X\n", ipbyte[1], ipbyte[1]);
    printf("IPBYTE[2] = %d = 0x%X\n", ipbyte[2], ipbyte[2]);
    printf("IPBYTE[3] = %d = 0x%X\n", ipbyte[3], ipbyte[3]);
    
    // big-endian -> el MSB va a la mayor direccion de memoria (ipbyte[3])
}
