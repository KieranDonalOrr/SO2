//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"
int main (int argc, char **argv) {

    //Declaraciones
    int bytesLeidos = 0, totalBytesLeidos = 0;
    unsigned int offset = 0;
    unsigned char tamBuffer[BLOCKSIZE*4];
    char descriptor[1024];

    //Inicializar buffers. 
    memset(tamBuffer, 0, sizeof(tamBuffer));

    //Comprobación de sintaxis 
    if(argc!=3){
        fprintf(stderr,"Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
		return -1;
    }

    //comprobación ruta correcta
    if((argv[2][strlen(argv[2])-1]) == '/'){
        fprintf(stderr,"Error: ruta no corresponde con fichcero mi_cat.c/nivel9\n");
        return -1;
    }

    strcpy(descriptor,argv[1]);
    
    
    bmount(argv[1]);
    bytesLeidos = mi_read(argv[2], tamBuffer, offset, sizeof(tamBuffer)); 
    //bucle que recorre todos los bloques
    while (bytesLeidos > 0){
        totalBytesLeidos += bytesLeidos;
        write(1, tamBuffer, bytesLeidos);
        //reseteamos buffer a 0
        memset(tamBuffer, 0, sizeof(tamBuffer)); 
        offset += sizeof(tamBuffer);
        bytesLeidos = mi_read(argv[2], tamBuffer, offset, sizeof(tamBuffer));
    }

    //comprobamos que el total de bytes leídos esté correcto
    if (totalBytesLeidos < 0) { 
        fprintf(stderr,"Error de ejecución en mi_cat.c/nivel9\n");
        return -1;
    }

    //Imprimimos por pantalla los bytes totales leídos
    fprintf(stderr, "Bytes leídos totales: %i\n", totalBytesLeidos);

    //Desmontamos dispositivo . 
    bumount();
    return 0;
}
