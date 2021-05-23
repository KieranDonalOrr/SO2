#include "directorios.h"

//Permite escribir texto en una posición de un fichero.de
int main(int argc, char **argv){



    const char *dir;
    int nBytesEsc;
    int nTextoEscrito = 0;
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

   

    //pedirá permisos de escritura, mi write llamará a mi_write_f
    //argv[3] corresponde con la cantidad de texto escrito
    nTextoEscrito = strlen(argv[3]);
    //argv[4] corresponde con la entrada del offset
    nBytesEsc = mi_write(argv[2], argv[3], atoi(argv[4]), nTextoEscrito);

    //testea que haya escrito bien
    if(nBytesEsc <0){
        fprintf(stderr, "Error de escritura nivel9. mi_escribir.c");
        
        return -1;
    }

    //antes de desmontar, mostrar la cantidad de bytes escritos
    printf("Cantidad de bytes escritos con mi_escribir: %d\n", nBytesEsc);

    bumount(dir);

    return 0;

    //no sé si falta algo, yo creo que esto es lo que pide.
}