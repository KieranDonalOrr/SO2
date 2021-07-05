//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"

//borra un fichero o directorio llamando a mi_unlink de la capa de directorios
int main(int argc, char **argv)
{
    
    bmount(argv[1]);
    mi_unlink(argv[2]);
    bumount();
    return 0;
}