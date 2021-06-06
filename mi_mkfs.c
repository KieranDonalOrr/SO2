//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "ficheros_basico.h"

int main (int argc, char **argv){
    if (argc!=3){

        printf("Mal uso del COMANDO: *nombre dispositivo* *numero de bloques*");

    }else{
        //Variable en la que guardamos el numero de bloques a guardar
        int nbloques;
        //Pasamos el string a un valor int.
        nbloques=atoi(argv[2]);

        //BMOUNT
        bmount(argv[1]);
        //BWRITE
        unsigned char buffer [BLOCKSIZE];
        //set de bits en buffer a 0.
        memset (buffer, '0', BLOCKSIZE);
        
        for (unsigned int i=1;i<nbloques;i++){
            bwrite(i,buffer);
            

        }
        initSB(nbloques, nbloques/4); 
        
       initMB(); 
       
        initAI();
        

        
        //nivel 3 creación del directorio raíz
        reservar_inodo('d', 7);

        //BUMOUNT
        bumount();


    }
}