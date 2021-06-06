//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL)
    {
        fprintf(stderr, "La sintaxis es incorrecta\n");
        return 0;
    }
    else
    {
        struct STAT p_stat;
        struct tm *tm;
        char tmp[100];
        bmount(argv[1]);
        int ninodo = mi_stat(argv[2], &p_stat);
        printf("Nº de inodo: %d\n", ninodo);
        printf("tipo: %c\n", p_stat.tipo);
        printf("permisos: %d\n", p_stat.permisos);
        tm = localtime(&p_stat.atime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900,
                tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        printf( "atime: %s\n", tmp);
        tm = localtime(&p_stat.ctime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900,
                tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        printf( "ctime: %s\n", tmp);
        tm = localtime(&p_stat.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900,
                tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        printf("mtime: %s\n", tmp);
        printf("nlinks: %d\n", p_stat.nlinks);
        printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);
        bumount();
        return 0;
        
    }
}