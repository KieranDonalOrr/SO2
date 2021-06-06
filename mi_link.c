//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"

int main(int argc, char **argv){
    if(argc != 4){
        fprintf(stderr, "La sintaxis correcta es incorrecta \n");
        return EXIT_FAILURE;
    }
    if (argv[2][strlen(argv[2]) - 1] != '/' && argv[3][strlen(argv[3]) - 1] != '/')
    {
    bmount(argv[1]);
    mi_link(argv[2], argv[3]);
    bumount();
    }else
    {
        fprintf(stderr, "No se admiten directorios\n");
        return -1;
    }
    return 0;
}
