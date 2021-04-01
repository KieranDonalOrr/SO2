#include "ficheros_basico.h"


//Devuelve el Tamaño en bloques del mapa de bits
int tamMB(unsigned int nbloques){
    
    //bits se agrupan en bloques de tamaño BLOCKSIZE
    int blocResult = (nbloques / 8);

    //operación para saber si necesitamos añadir un bloque adicional
    if((blocResult % BLOCKSIZE) !=0){

        return ((blocResult/ BLOCKSIZE)+1);
    }
    else{
        return (blocResult / BLOCKSIZE);
        
    }
}
//Calcula el tamaño en bloques del array de inodos
int tamAI(unsigned int ninodos){

//el sistema de ficheros utiliza ninodos = nbloques/4 <- mi_mkfs pasará el dato a esta función como parámetro

    int resultTamAI = ((ninodos * INODOSIZE)/ BLOCKSIZE);

    if((resultTamAI % BLOCKSIZE) != 0){
        return (resultTamAI+1);

    }
    else{
        return resultTamAI;
        
    }


}