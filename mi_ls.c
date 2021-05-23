#include "directorios.h"
#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) 

int main (int argc, char **argv){
    if (argv[1] == NULL || argv[2] == NULL)
    {
        fprintf(stderr, "La sintaxis es incorrecta\n");
        return 0;
    }
    char *buffer = malloc(TAMBUFFER);
    memset(buffer, 0, sizeof(char) * TAMBUFFER);
    if((mi_dir(argv[2], buffer)) == -1){
        bumount();
        return -1;
    }
    fprintf(stderr, "Nombre\n");
    fprintf(stderr, "----------------\n%s", buffer);
    bumount();
    return 0;

}