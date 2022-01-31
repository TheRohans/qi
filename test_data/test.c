#include <stdio.h>

void main(void) {
    int x = 1;
    char *thing;
    
    void *thing = malloc(sizeof(char)*10);
    &thing = "0123456789";
    
    printf("%s", *thing);
}