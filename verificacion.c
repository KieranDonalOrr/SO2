//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "verificacion.h"

int main(int argc, char **argv){


    struct STAT stat;
    int numentradas;
    int error;
    char *ds = argv[2];
    char f[50];

    //comprobar sintaxis
    if(argc != 3){
        fprintf(stderr,"/Uso: verificacion <nombre_dispositivo> <directorio_simulación\n");
        return -1;
    }
    //montar dispositivo
    bmount(argv[1]);

    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    error = mi_stat(ds, &stat); //el directorio simulación corresponderá al &argv[2]
     if(error < 0){
         fprintf(stderr, "Error en mi_stat, verificación.c /nivel13\n");
        return -1;
     }
     numentradas= stat.tamEnBytesLog / sizeof(struct entrada);
     printf("Número entradas del directorio simulación %s, numentradas: %i, nprocesos: %d\n", ds, numentradas,NUMPROCESOS);

    if(numentradas != NUMPROCESOS){

        fprintf(stderr, "ERROR: El numero entradas no corresponde con el numero de procesos, verificacion.c/nivel13");
        return -1;
    }
    
    //crear el fichero "informe.txt" dentro del directorio de simulación
    sprintf(f,"%sinforme.txt", ds );

    error= mi_creat(f, 7);
    if (error < 0){

        fprintf(stderr, "ERROR en mi_creat, verificacion.c/nivel13\n");
        return -1;
    }

    //Leer los directorios correspondientes a los procesos  
    //Mejora: Las entradas también las podéis haber leído todas de golpe previamente al inicio del bucle, 
    //con una sola llamada a mi_read() utilizando un buffer del tamaño NUMPROCESOS * sizeof (struct entrada) o llamando a vuestra función mi_dir() en su versión simple .
    struct entrada ent;
    char bufferEntrada[NUMPROCESOS * sizeof(struct entrada)]; //honestamente hago la mejora porque adelaida me dice como se hace
    char prueba[];
    //inicializo buffer a 0
    memset(bufferEntrada, 0,NUMPROCESOS * sizeof(struct entrada));
    //leer entrada del buffer
    error = mi_read(ds, &bufferEntrada, 0,NUMPROCESOS * sizeof(struct entrada));
    //test mi_read
    if(error < 0){
        fprintf(stderr,"Error de lectura buffer, verificacion.c/nivel13\n");
        return -1;
    }
    
    struct INFORMACION info; 

    for( int i = 0; i < numentradas; i++){

    //Podéis utilizar la función strchr() con el carácter  '_'  para obtener los caracteres del PID, 
    //y luego pasarlos a entero con la función atoi(). El registro info es de tipo struct INFORMACION.
    char aux;
    aux = strchr(ent.nombre,'_');
    info.pid = atoi(aux+ 1);

    //Recorrer secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras: 
     int cant_registros_buffer_escrituras = 256; 
 struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
 memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
 while (mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) {

    }




    }


}