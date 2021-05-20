#include "directorios.h"

//Permite escribir texto en una posición de un fichero.de
int main(int argc, char **argv){

struct STAT STAT;
struct superbloque SB;
    dir = argv[1];
    dir=argv[1];   
    bmount(dir); 
    
    //control de sintaxis
    if (argc != 5)
    {
        fprintf(stderr, " Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset> \n");
        return -1;
    }

    //recibe como parámetro la ruta del fichero
    if(argv[2][strlen(argv[2]) -1]== '/'){

        fprintf( stderr, "Ruta introducida no es de un fichero\n");
        return -1;
    }

    //no está terminado



    return 0;
}