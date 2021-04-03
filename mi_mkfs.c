#include "bloques.h"


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
        //BUMOUNT
        bumount();


    }
}