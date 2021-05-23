#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "La sintaxis es incorrecta\n");
        return -1;
    }
    bmount(argv[1]);
    mi_unlink(argv[2]);
    bumount(argv[1]);
    return 0;
}