//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"

int main(int argc, char **argv)
{
    
    bmount(argv[1]);
    mi_unlink(argv[2]);
    bumount();
    return 0;
}