#include "ficheros.h"

//escribirá un teto en uno o varios inodos haciendo uso de reservar_inodo('f',6) para obtener un nInodo que se mostrará por pantalla
//y será utilizado como parámetro
int main(int argc, char **argv)
{
    struct STAT STAT;
    struct superbloque SB;
    int nBytes = 0;
    const char *dir;
    //ejemplos de offset para utilizar los diferentes tipos de punteros
    unsigned int offset[5] = {9000, 209000, 30725000, 409605000, 480000000};
    unsigned int l = strlen(argv[2]);
    char *buffer = malloc(l);
    printf("longitud del texto %d \n", l);
    //creamos y mostramos nInodo
    unsigned int nInodo;
    unsigned int diferentes_inodos = atoi(argv[3]);
    dir = argv[1];
    dir=argv[1];   
    bmount(dir); 
    bread(posSB, &SB);
    
    

    //control de sintaxis
    if (argc != 4)
    {
        fprintf(stderr, "escribir <nombre_dispositivo> <$(cat fichero)> <diferentes_inodos> \n"
                        " Offsets: 9000, 209000, 30725000, 409605000, 480000000\n"
                        "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
        return -1;
    }

    

    //definimos buffer en el que introduciremos un texto
    
    if (diferentes_inodos == 0)
    {
        nInodo = reservar_inodo('f', 6);
        strcpy(buffer, argv[2]);
        
        
        //bucle que itera 5 veces para recorrer todos los offset
        for (int i = 0; i < 5; i++)
        {
            fprintf(stderr,"Nº inodo reservado: %d\n", nInodo);
            fprintf(stderr,"offset:  %d\n", offset[i]);

            nBytes = mi_write_f(nInodo, buffer, offset[i], l);
            
            fprintf(stderr,"Los bytes escritos por mi_write_f han sido: %d\n", nBytes);
            mi_stat_f(nInodo, &STAT);
            fprintf(stderr,"stat.tamEnBytesLog=%d\n",STAT.tamEnBytesLog);
            fprintf(stderr,"stat.numBloquesOcupados=%d\n",STAT.numBloquesOcupados);
           
        }
        fprintf(stderr,"final inodos=0\n");
    }
    else if (diferentes_inodos == 1)
    {
        nInodo = reservar_inodo('f', 6);
        strcpy(buffer, argv[2]);
        fprintf(stderr,"inodos=0\n");
        //bucle que itera 5 veces para recorrer todos los offset
        for (int i = 0; i < 4; i++)
        {
            fprintf(stderr,"Nº inodo reservado: %d\n", nInodo);
            fprintf(stderr,"offset:  %d\n", offset[i]);

            nBytes = mi_write_f(nInodo, buffer, offset[i], l);
            fprintf(stderr,"Los bytes escritos por mi_write_f han sido: %d\n", nBytes);
            mi_stat_f(nInodo, &STAT);
            fprintf(stderr,"stat.tamEnBytesLog=%d\n",STAT.tamEnBytesLog);
            fprintf(stderr,"stat.numBloquesOcupados=%d\n",STAT.numBloquesOcupados);
           
        }
        fprintf(stderr,"final inodos=0\n");
    }
    bumount(dir);
    return 1;
}