#include "directorios.h"

//comando que muestra todo el contenido de un fichero
//Sintaxis: ./mi_cat <disco> </ruta_fichero>
int main(int argc, char **argv){

  //control de sintaxis
    if(argc != 3){
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero> \n");
        return -1;
    }
    
       //recibe como parámetro la ruta del fichero
    if(argv[2][strlen(argv[2]) -1]== '/'){

      fprintf( stderr, "Ruta introducida no es de un fichero\n");
      return -1;
    }

    const char *direccion;
    direccion =argv[1];
    int nBytes =1500;
    int offset=0;
    char *tambuffer[nBytes];
    int cantidadLeido=0;

    bmount(direccion);
    memset(tambuffer, 0, nBytes);

    //los bytes leídos han de coincidir con el tamaño en bytes lógicos
    cantidadLeido = mi_read(argv[2],tambuffer,offset, nBytes);
    if(cantidadLeido == -1){

      fprintf(stderr, "Error de lectura nivel9, mi_cat.c\n");
      return -1;
    }
    printf("cantidad de bytes leidos es: %d",cantidadLeido);

    bumount();

    return 0;  
  }