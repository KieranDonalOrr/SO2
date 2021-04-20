#include "ficheros.h"

//escribirá un teto en uno o varios inodos haciendo uso de reservar_inodo('f',6) para obtener un ninodo que se mostrará por pantalla 
//y será utilizado como parámetro
int escribir(int argc, char **argv){

    bmount(argv[1]);
    //ejemplos de offset para utilizar los diferentes tipos de punteros
    unsigned int offset[5] = {9000, 209000, 30725000, 409605000, 480000000};

    int ninodo = reservar_inodo('f',6);

    printf(" el numero de inodo reservado: %d\n", ninodo);

    char buffer[BLOCKSIZE*2];

    strcpy(buffer, "Mi campeón favorito, sin duda alguna Anivia, la criofenix. No tendrá mucho ataque, no tendrá mucha defensa... "
                    "pero me encanta. Es muy difícil de controlar, sobre todo la Q, porque tienes que petarla y controlar la distancia "
                    "muy bien para stunear.");


}