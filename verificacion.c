#include "verificacion.h"

int main(int argc, char **argv){


    struct STAT stat;
    int numentradas;
    int error;
    char *ds = argv[2];

    //comprobar sintaxis
    if(argc != 3){
        fprintf(stderr,"/Uso: verificacion <nombre_dispositivo> <directorio_simulación\n");
        return -1;
    }
    //montar dispositivo
    bmount(argv[1]);

    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    error = mi_stat(ds, &stat); //el directorio simulación corresponderá al &argv[2]
     if(error < 0){
         fprintf(stderr, "Error en mi_stat, verificación.c /nivel13\n");
        return -1;
     }
     numentradas= stat.tamEnBytesLog / sizeof(struct entrada);
     printf("Número entradas del directorio simulación %s, numentradas: %i, nprocesos: %d\n", ds, numentradas,NUMPROCESOS);

    if(numentradas != NUMPROCESOS){

        fprintf(stderr, "ERROR: El numero entradas no corresponde con el numero de procesos, verificacion.c/nivel13");
        return -1;
    }

}