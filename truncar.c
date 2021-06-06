//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "ficheros.h"


int main(int argc, char **argv){
    if(argv[1]==NULL){
        printf("\nSintaxis: primer arg\n");
        return -1;
    }
    if(argv[2]==NULL){
        printf("\nSintaxis: segundo arg\n");
        return -1;
    }
    if(argv[3]==NULL){
        printf("\nSintaxis: tercer arg\n");
        return -1;
    }

    const char *dir;
    dir = argv[1];
    int nInodo = atoi(argv[2]);
    int nBytes = atoi(argv[3]);
    bmount(dir);
    struct STAT STAT;
    struct tm * info;
    char aFecha[24], cFecha[24], mFecha[24];
    if(nBytes==0){
      liberar_inodo(nInodo);
    }
    else{
       mi_truncar_f(nInodo,nBytes);
    }
    mi_stat_f(nInodo, &STAT);
        bumount(dir);
        strftime(aFecha, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&STAT.atime));
        strftime(cFecha, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&STAT.ctime));
        strftime(mFecha, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&STAT.mtime));
        fprintf(stderr, "DATOS INODO %i\n\
        tipo=%c\n\
        permisos=%i\n\
        atime: %s\n\
        ctime: %s\n\
        mtime: %s\n\
        nlinks: %i\n\
        tamEnBytesLog=%i\n\
        numBloquesOcupados=%i\n", nInodo, STAT.tipo, STAT.permisos, aFecha, cFecha, mFecha,STAT.nlinks, STAT.tamEnBytesLog, STAT.numBloquesOcupados);
        bumount();
    return 1;

}