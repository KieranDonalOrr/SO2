#include "directorios.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    {
        fprintf(stderr, "Sintaxis incorrecta\n");
        bumount(argv[1]);
        return 0;
    }
    else
    {
    
        int permisos = atoi(argv[2]);
        if (permisos < 0)
        {
            fprintf(stderr, "Los permisos son menores a 0\n");
            bumount(argv[1]);
            return 0;
        }
        else if (permisos > 7)
        {

            fprintf(stderr, "Los permisos son mayores a 7\n");
            bumount(argv[1]);
            return 0;
        }
        else
        {
            bmount(argv[1]);
            mi_creat(argv[3], atoi(argv[2]));
            bumount(argv[1]);
            return 0;
        }
    }
}