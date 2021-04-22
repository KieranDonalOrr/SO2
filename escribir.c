#include "ficheros.h"

//escribirá un teto en uno o varios inodos haciendo uso de reservar_inodo('f',6) para obtener un ninodo que se mostrará por pantalla
//y será utilizado como parámetro
int escribir(int argc, char **argv)
{
    //control de sintaxis
    if(argc != 4){
        fprintf(stderr, "escribir <nombre_dispositivo> <$(cat fichero)> <diferentes_inodos> \n"
                        " Offsets: 9000, 209000, 30725000, 409605000, 480000000\n"
                        "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
        return -1;
    }

    struct stat stat;

    bmount(argv[1]);
    //ejemplos de offset para utilizar los diferentes tipos de punteros
    unsigned int offset[5] = {9000, 209000, 30725000, 409605000, 480000000};

    //creamos y mostramos ninodo
    unsigned int ninodo = reservar_inodo('f', 6);

    // Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets
    //utilizo atoi para convertir el string en un valor numérico
    unsigned int diferentes_inodos = atoi(argv[3]);

    int l = strlen(argv[2]);
    printf("longitud del texto %d \n", l);

    //definimos buffer en el que introduciremos un texto
    char buffer[BLOCKSIZE * 2];

    strcpy(buffer, "Mi campeón favorito, sin duda alguna Anivia, la criofenix. No tendrá mucho ataque, no tendrá mucha defensa... "
                   "pero me encanta. Es muy difícil de controlar, sobre todo la Q, porque tienes que petarla y controlar la distancia "
                   "muy bien para stunear.");

    unsigned int nbytes = strlen(buffer);
    //bucle que itera 5 veces para recorrer todos los offset
    for (int i = 0; i < 5; i++)
    {
        //si diferentes inodos = 0 se reserva un solo inodo para todos los offset
        if (diferentes_inodos != 0)
        {
            ninodo = reservar_inodo('f', 6);
        }
        printf("Nº inodo reservado: %d\n", ninodo);
        printf("offset:  %d\n", offset[i]);

        
       // [traducir_bloque_inodo()→ inodo.punterosDirectos[8] = 3139 (reservado BF 3139 para BL 8)]
        fprintf(stderr, "");
    
        int write = mi_write_f(ninodo, *buffer, offset[i], nbytes);
        printf("Los bytes escritos por mi_write_f han sido: %d", write);
        //reseteamos el buffer poniéndolo a 0
        memset(buffer, 0, (BLOCKSIZE * 2));
        int read = mi_read_f(ninodo, *buffer, offset[i], nbytes);
        printf("Los bytes leídos por mi_read_f han sido %d ", read);

        mi_stat_f(ninodo, &stat);

        //haha faltan cosas.
        //utilizar mi_stat_f CREO uns aludo hay que pasarle el inodo al mi stat f y te suelta los metadatos
    }
}