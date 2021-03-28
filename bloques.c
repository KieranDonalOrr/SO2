#include "bloques.h"
//Descriptor de fichero
static int descriptor=0;
//FUNCION QUE MONTA EL DISPOSITIVO VIRTUAL
int bmount(const char *camino){
    umask(000);

    descriptor=open(camino, O_RDWR|O_CREAT,0666);
    if(descriptor==-1){
        fprintf(stderr, "ERROR %d: %s\n",errno,strerror(errno));
        return -1;
    }else{
        return descriptor;
    }

}
//FUNCION QUE DESMONTA EL DISPOSITIVO VIRTUAL
int bumount(){
    int resultado;
    resultado=close(descriptor);
    if(resultado==-1){
        fprintf(stderr,"ERROR %d: %s\n",errno,strerror(errno)); 
        return -1;

    }else{
        return 0;
    }
}

int bwrite(unsigned int nbloque, const void *buf){
    off_t desplazamiento= nbloque * BLOCKSIZE;
    int punto_de_referencia= SEEK_SET;
    off_t lseek(int descriptor, off_t desplazamiento, int punto_de_referencia);
    ssize_t write(int descriptor, const void *buf, size_t nbytes);

}

int bread(unsigned int nbloque, void *buf){


}