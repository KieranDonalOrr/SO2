//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "ficheros.h"


int main( int argc, char** argv){

    //control de sintaxis
    if(argc != 3){
        fprintf(stderr, "Sintaxis: ");
    }
    const char *direccion;
    direccion =argv[1];
    int nInodo= atoi(argv[2]);
    struct inodo inodoAux;
    int finalDeFichero=0;
    int nBytes =1500;
    int offset=0;
    const void *bufferTexto[nBytes];
    int cantidadLeido=0;
    int aux=1;


    bmount(direccion);
    leer_inodo(nInodo,&inodoAux);
    while (inodoAux.tipo!='l'&& finalDeFichero==0){
        memset(bufferTexto, 0, nBytes);
        cantidadLeido= cantidadLeido+mi_read_f(nInodo, bufferTexto, offset, nBytes);
        aux=mi_read_f(nInodo, bufferTexto, offset, nBytes);
        if(aux==-1){
          bumount();
          return -1; 
        }
        if(aux<nBytes){
           const char *bufferAux[aux];
                memcpy(bufferAux, bufferTexto, aux);
                write(1, bufferAux, aux); 
        }else{
            write(1,bufferTexto,aux);
        }
        if(aux==0){
          finalDeFichero=1; 
        }
        offset=offset+aux;
    }
    
        fprintf(stderr, "\n,>El total de bytes leídos: %i\n", cantidadLeido);
        return 1;
    
    
}