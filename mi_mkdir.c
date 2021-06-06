//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    {
        fprintf(stderr, "Sintaxis incorrecta\n");
        bumount();
        return 0;
    }
    else
    {
        int length = strlen(argv[3]);
        if (argv[3][length - 1] != '/')
        {
            fprintf(stderr, "No es un directorios es un fichero\n");
            bumount();
            return 0;
        }
        else
        {
            int permisos = atoi(argv[2]);
            if (permisos < 0)
            {
                fprintf(stderr, "Los permisos son menores a 0\n");
                bumount();
                return 0;
            }
            else if (permisos > 7)
            {

                fprintf(stderr, "Los permisos son mayores a 7\n");
                bumount();
                return 0;
            }
            else
            {
                bmount(argv[1]);

                if (mi_creat(argv[3], permisos) == 0)
                {
                    bumount();
                    return 0;
                }
            }
        }
    }
}