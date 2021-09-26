#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<netinet/in.h>
#include<arpa/inet.h>

// **********************************************
// PASO BINARIO -> TEXTUAL CON IPv6
// **********************************************

int main(){
    struct in6_addr ipbin;
    char iptext[INET6_ADDRSTRLEN];
    int error;
    
    ipbin.s6_addr32[0] = 0x01234567;
    ipbin.s6_addr32[1] = 0x89ABCDEF;
    ipbin.s6_addr32[2] = 0x01234567;
    ipbin.s6_addr32[3] = 0x89ABCDEF;
    
    if(inet_ntop(AF_INET6, (const void *) &ipbin, iptext, INET6_ADDRSTRLEN) == NULL){
      fprintf(stderr, "Error en la conversión de binario a textual.\n");
      exit(EXIT_FAILURE);
    }
    
    printf("IPTEXT = %s\n", iptext);
    
    return(EXIT_SUCCESS);   
}
