#include "bloques.h"
//Descriptor de fichero
static int descriptor = 0;
off_t lseek(int descriptor, off_t desplazamiento, int punto_de_referencia);
ssize_t write(int descriptor, const void *buf, size_t nbytes);
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
//Escribe 1 bloque en el dispositivo virtual, especificado por nbloque
int bwrite(unsigned int nbloque, const void *buf){
    off_t desplazamiento= nbloque * BLOCKSIZE;
    int punto_de_referencia= SEEK_SET;
    lseek(descriptor,desplazamiento, punto_de_referencia);
    int i = write(descriptor, buf, BLOCKSIZE);
    if (i==-1){
       return EXIT_FAILURE;
    }else{
        return BLOCKSIZE;
    }

}
//Lee 1 bloque del dispositivo virtual, correspondiente al bloque especificado por nbloque
int bread(unsigned int nbloque, void *buf){
    off_t desplazamiento= nbloque * BLOCKSIZE;
    int punto_de_referencia= SEEK_SET;
    lseek(descriptor,desplazamiento, punto_de_referencia);
    int i = read(descriptor, buf, BLOCKSIZE);
    if (i==-1){
       return EXIT_FAILURE;
    }else{
        return BLOCKSIZE;
    }


}