//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
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
    int tamBuffer = BLOCKSIZE * 4;
    char *lectura = malloc(tamBuffer);
    int offset=0;
    
    int cantidadLeido=0;

    bmount(direccion);
   

    //los bytes leídos han de coincidir con el tamaño en bytes lógicos
    cantidadLeido = mi_read(argv[2],lectura,offset,  BLOCKSIZE * 4);
    if(cantidadLeido == -1){
      fprintf(stderr, "Error de lectura nivel9, mi_cat.c\n");
      return -1;
    }
    fprintf(stderr,"cantidad de bytes leidos es: %d\n",cantidadLeido);

    bumount();

    return 0;  
  }