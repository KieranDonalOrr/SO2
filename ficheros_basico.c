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

//Inicializa los datos del superbloque
int initSB(unsigned int nbloques, unsigned int ninodos){

    struct superbloque SB;
    //primer bloque mapa bits
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB =1
    //ultimo bloque mapa bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    //primer bloque array inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    //último bloque array inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    //primer bloque datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    //ultimo bloque datos
    SB.posUltimoBloqueDatos = nbloques-1;
    //inodo directorio raíz
    SB.posInodoRaiz = 0;
    //primer inodo libre en array inodo
    SB.posPrimerInodoLibre = 0;
    //cantidad bloques libres en SF
    SB.cantBloquesLibres = nbloques;
    //cantidad inodos libres en array de inodos
    SB.cantInodosLibres = ninodos;
    //total de bloques
    SB.totBloques = nbloques;
    //total de inodos
    SB.totInodos = ninodos;

    //escribimos estructura en en posSB
    int escrituraBl = bwrite(posSB, &SB);

}

//inicializa el mapa de bits (en nivel 2 simplemente se pone todo a 0)
int initMB(){


    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);

    //contenido buffer se escribe en los bloques del mapa de bits
    
}