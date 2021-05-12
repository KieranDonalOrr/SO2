#include "ficheros_basico.h"
#include "bloques.h"
#include <time.h>
#define DEBUG 1
struct superbloque SB;
struct inodo inodos[BLOCKSIZE / INODOSIZE];
//Devuelve el Tamaño en bloques del mapa de bits
int tamMB(unsigned int nBloques)
{

    //bits se agrupan en bloques de tamaño BLOCKSIZE
    int blocResult = (nBloques / 8);

    //operación para saber si necesitamos añadir un bloque adicional
    if ((blocResult % BLOCKSIZE) != 0)
    {

        return ((blocResult / BLOCKSIZE) + 1);
    }
    else
    {
        return (blocResult / BLOCKSIZE);
    }
}
//Calcula el tamaño en bloques del array de inodos
int tamAI(unsigned int numInodos)
{

    //el sistema de ficheros utiliza numInodos = nBloques/4 <- mi_mkfs pasará el dato a esta función como parámetro

    int resultTamAI = ((numInodos * INODOSIZE) / BLOCKSIZE);

    if ((resultTamAI % 1) != 0)
    {
        return (resultTamAI);
    }
    else
    {
        return resultTamAI;
    }
}

//Inicializa los datos del superbloque
int initSB(unsigned int nBloques, unsigned int numInodos)
{

    //primer bloque mapa bits
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB =1
    //ultimo bloque mapa bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nBloques) - 1;
    //primer bloque array inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    //último bloque array inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(numInodos) - 1;
    //primer bloque datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    //ultimo bloque datos
    SB.posUltimoBloqueDatos = nBloques - 1;
    //inodo directorio raíz
    SB.posInodoRaiz = 0;
    //primer inodo libre en array inodo
    SB.posPrimerInodoLibre = 0;
    //cantidad bloques libres en SF
    SB.cantBloquesLibres = nBloques;
    //cantidad inodos libres en array de inodos
    SB.cantInodosLibres = numInodos;
    //total de bloques
    SB.totBloques = nBloques;
    //total de inodos
    SB.totInodos = numInodos;

    //escribimos estructura en en posSB
    bwrite(posSB, &SB);
    return -1;
}

//inicializa el mapa de bits (en nivel 2 simplemente se pone todo a 0)
int initMB()
{
    void *buf[BLOCKSIZE];
    int bloque = SB.posPrimerBloqueDatos;
    //preparamos un buffer tamaño BLOCKSIZE
    memset(buf, 0, sizeof(buf)); //Control
    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++)
    {
        if (bwrite(i, buf) == -1) // iterando por todos los bloques de MB y llenandolos
        {
            fprintf(stderr, "Error de escritura en initMB, ficheros_basico.c \n");
            return -1;
        }
    }
    //Limpiamos el mapa
    for (int i = 0; i < SB.cantBloquesLibres; i++)
    {
        bwrite(bloque, buf);
        bloque++;
    }

    for (int i = 0; i < SB.posPrimerBloqueDatos; i++)
    { // Reservar todos los bloques que no son DATOS
        reservar_bloque();
    }
    return 0;
}

//inicializar lista de inodos libres
int initAI()
{
    int done = 0;
    //obtengo dirección array inodos
    bread(posSB, &SB);
    //se pone a 0 los inodos.
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    //si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && (done != 1); i++)
    {
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            inodos[j].tipo = 'l'; //libre
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            else
            { //hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                done = 1; // Nos permite salir del for anidado completamente

                //hay que salir del bucle, el último bloque no tiene por qué estar completo
                break;
            }
        }
        //escribir el bloque de inodos en el dispositivo virtual
        bwrite(i, &inodos);
        memset(inodos, '0', sizeof(struct inodo) * 8); // control de errores
    }
    return -1;
}
//escribe el valor indicado por el parametro bit (0 libre, 1 ocupado) en un determinado bloque
int escribir_bit(unsigned int nBloque, unsigned int bit)
{
    unsigned char *bufferMB = malloc(BLOCKSIZE);
    unsigned char mascara = 128;
    struct superbloque SB;
    int poSByte;
    int poSBit;
    int poSBlock;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error in escribir_bit\n");
        return -1;
    }
    //calculamos la posición del byte en el MB
    poSByte = nBloque / 8;
    poSBit = nBloque % 8;
    //posicion absoluta del dispositivo virtual en el que se encuentra el bloque
    poSBlock = (poSByte / BLOCKSIZE) + SB.posPrimerBloqueMB;
    if (bread(poSBlock, bufferMB) == -1)
    {
        printf("Error en leyendo el bloque en escribir bit\n");
        return -1;
    }
    //utilizamos mascara para desplazar bits a la derecha
    mascara >>= poSBit;
    //localizamos posición de poSByte
    poSByte = poSByte % BLOCKSIZE;
    //instrucción para poner a 0 o a 1 el bit correspondiente
    if (bit == 1)
    {
        bufferMB[poSByte] |= mascara;
    }
    else if (bit == 0)
    {
        bufferMB[poSByte] &= ~mascara;
    }
    //escribimos buffer del MB en dispositivo virtual
    if (bwrite(poSBlock, bufferMB) == -1)
    {
        printf("Error en escribir el bloque\n");
        return -1;
    }
    free(bufferMB);
    return 0;
}
//lee determinado bit del MB y devuelve valor del bit leído
//misma operativa que en la función anterior
unsigned char leer_bit(unsigned int nBloque)
{
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;
    struct superbloque SB;
    long poSByte, posByte;
    int poSBit;
    int poSBlock;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    //se calcula la posición del byte en el MB
    poSByte = nBloque / 8;
    poSBit = nBloque % 8;
    //posición absoluta del dispositivo virtual en el que se encuentra el bloque
    poSBlock = (poSByte / BLOCKSIZE) + SB.posPrimerBloqueMB;
    //set del bufferMB a 0
    memset(bufferMB, 0, BLOCKSIZE);
    //hacemos un bread utilizando el bufferMB
    if (bread(poSBlock, bufferMB) == -1)
    {
        printf("Error en leer el bloque\n");
        return -1;
    }

    posByte = poSByte % BLOCKSIZE;
    mascara >>= poSBit;
    mascara &= bufferMB[posByte];
    mascara >>= (7 - poSBit);
#if DEBUG
    printf("\n[leer_bit(%d)→ posbyte:%ld, posbit:%d, nbloqueMB:%ld, nbloqueabs:%ld)] leer_bit(%d) = %d\n", nBloque, poSByte, poSBit, poSByte / BLOCKSIZE, SB.posPrimerBloqueMB + (poSByte / BLOCKSIZE), nBloque, mascara);
#endif
    return mascara;
}
//encuentra primer bloque libre, lo ocupa y devuelve su posición
int reservar_bloque()
{
    //definición de todos los parámetros que necesitaremos
    unsigned char buffer[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    int bloqueMB;
    unsigned int numBloque;
    int poSByte;
    int poSBit;
    unsigned char mascara = 128;
    //leemos superbloue para obtener su correcta localización
    if (bread(posSB, &SB) == -1)
    {
        return EXIT_FAILURE;
    }
    //comprobamos si quedan bloques libres
    if (SB.cantBloquesLibres > 0)
    {
        //buffer auxiliar que utilizaremos para comparar cada bloque inicializado a 1
        memset(bufferAux, 255, BLOCKSIZE);
        //localizamos posicion primer bloque del MB
        bloqueMB = SB.posPrimerBloqueMB;

        //leemos mapa de bits utilizando el buffer seteado a 1
        if (bread(bloqueMB, buffer) == -1)
        {
            printf("Error en leer el mapa de bits\n");
            return -1;
        }
        //bucle que compara los bits de ambos buffer
        while (memcmp(bufferAux, buffer, BLOCKSIZE) == 0)
        {
            //miramos que la posición leída del bloque sea menor o igual al último bloque
            //indicando que hay bloques libres
            if (bloqueMB <= SB.posUltimoBloqueMB)
            {
                //actualizamos valor del bloque de mapa de bits
                bloqueMB++;
                //leemos mapa de bits utilizando el buffer
                if (bread(bloqueMB, buffer) == -1)
                {
                    printf("Error error en leer el mapa de bits en bucle\n");
                    return -1;
                }
            }
            //si no hay bloques libres saltará error y saldremos de la función
            else
            {

                printf("Error no hay bloques libres\n");
                return -1;
            }
        }
        //recorremos bucle buscando el primer 0
        for (poSByte = 0; buffer[poSByte] == 255; poSByte++)
        {
        }

        //el bit encontrado debe corresponder al primer bloque libre
        //comprobamos que esté dentro del tamaño del mapa de bits
        if (buffer[poSByte] != 255)
        {
            poSBit = 0;
            //buscamos en qué posición del bit está el 0
            while (buffer[poSByte] & mascara)
            {
                //desplazamiento de bits a la izquierda
                buffer[poSByte] <<= 1;
                poSBit++;
            }
        }

        //cálculo para determinar finalmente el nº de bloque
        numBloque = ((bloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + poSByte) * 8 + poSBit;

        //utilizamos la función escribir_bit pasándole un 1 y el numero calculado para indicar
        //que el bloque está reservado
        if (escribir_bit(numBloque, 1) != -1)
        {
            //decrementamos cantidad de bloques libres
            SB.cantBloquesLibres--;
            //guardamos el superbloque actualizado
            if (bwrite(posSB, &SB) == -1)
            {
                printf("Error al escribir en el superbloque\n");
                return -1;
            }
            //devolvemos el numero de bloque reservado
            return numBloque;
        }
        else
        {

            printf("Error al escribir el bit\n");
            return -1;
        }
    }
    //si el bit encontrado no corresponde con el mapa de bits
    //es porque no hay bloques libres y por tanto devolvemos error
    else
    {
        printf("Error no hay bloques libres\n");
        return -1;
    }
}
//libera un bloque determinado
int liberar_bloque(unsigned int nBloque)
{
    struct superbloque SB;

    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    //ponemos a 0 el bit del MB correspondiente al nbloque recibido por parametro
    escribir_bit(nBloque, 0);
    if (SB.cantBloquesLibres < SB.totBloques)
    {
        //incrementamos cantidad de bloques libres
        SB.cantBloquesLibres++;
        escribir_bit(nBloque, 0);
    }
    else
    {
        printf("Todos los bloques estan libres");
    }
    //devolvemos el bloque liberado
    return nBloque;
}

//escribe el contenido de una variable struct inodo en un inodo específico del array de inodos
int escribir_inodo(struct inodo inodo, unsigned int numInodo)
{
    struct superbloque SB;
    int nBloque;
    struct inodo ai[BLOCKSIZE / (BLOCKSIZE / 8)];
    //leemos superbloque para obtener dirección array inodos
    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    nBloque = SB.posPrimerBloqueAI + (numInodo / (BLOCKSIZE / (BLOCKSIZE / 8)));

    if (bread(nBloque, ai) == -1)
    {
        printf("Error en leer el inodoc\n");
        return -1;
    }
    ai[(numInodo % (BLOCKSIZE / (BLOCKSIZE / 8)))] = inodo;

    //escribimos el bloque modificado en el dispositivo virtual
    if (bwrite(nBloque, ai) == -1)
    {
        printf("Error en escribir el inodo\n");
        return -1;
    }
    return nBloque;
}
//lee un determinado inodo del array de inodos y lo vuelva en una variable struct inodo
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{

    struct superbloque SB;
    int nBloque;
    struct inodo ai[BLOCKSIZE / (BLOCKSIZE / 8)];
    //leemos superbloque para localizar array de inodos
    if (bread(posSB, &SB) == -1)
    {
        printf("Error al leer el superbloque\n");
        return -1;
    }
    //nBloque del array de inodos que tiene el inodo solicitado
    nBloque = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / (BLOCKSIZE / 8)));
    if (bread(nBloque, &ai) == -1)
    {
        printf("Error al leer el inodo\n");
        return -1;
    }
    //inodo solicitado se almacena en la siguiente posición
    *inodo = ai[ninodo % (BLOCKSIZE / INODOSIZE)];

    //si ha ido todo bien devolvemos 0
    return 0;
}

//encuentra el primer inodo libre, lo reserva, devuelve su número y actualiza la lista enlazada de inodos libres
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodo;
    int numInodo;
    //leemos superbloque para localizar array de inodos.
    if (bread(posSB, &SB) == -1)
    {
        printf("Error al leer el superbloque\n");
        return -1;
    }
    //comprobamos si hay inodos libres
    if (SB.cantInodosLibres > 0)
    {
        //inicialización de todos los campos del inodo al que apuntaba inicialmente al superbloque
        //time_t now;
        //desde que se ha cambiado el tipo de estructura que es leer_inodo no se puede hacer este tipo de asignación
        leer_inodo(SB.posPrimerInodoLibre, &inodo);
        numInodo = SB.posPrimerInodoLibre;
        SB.posPrimerInodoLibre = inodo.punterosDirectos[0];
        inodo.tipo = tipo;
        inodo.permisos = permisos;
        inodo.atime = time(NULL);
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.nlinks = 1;
        inodo.tamEnBytesLog = 0;
        inodo.numBloquesOcupados = 0;
        int i;
        for (i = 0; i < 12; i++)
        {
            inodo.punterosDirectos[i] = 0;
        }

        for (; i < 3; i++)
        {
            inodo.punterosIndirectos[i] = 0;
        }
        //escribimos inodo inicializado en la posición del primer inodo libre
        escribir_inodo(inodo, numInodo);
        //actualizamos cantidad de inodos libres
        SB.cantInodosLibres = SB.cantInodosLibres - 1;
        //escribimos en superbloque
        if (bwrite(posSB, &SB) == -1)
        {
            printf("Error en escribir en el superbloque\n");
            return -1;
        }
        //devolvemos la posicion del inodo reservado
        return numInodo;
    }
    //indicamos error y salimos en caso de no haber inodos libres
    else
    {
        printf("Error no hay inodos libres\n");
        return -1;
    }
}
//rango de punteros en el que se sitúa el bloque lógico que se pretende buscar
int obtener_nRangoBL(struct inodo inodo, unsigned int nblogico, unsigned int *ptr)
{

    if (nblogico < DIRECTOS)
    {
        *ptr = inodo.punterosDirectos[nblogico];

        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    }

    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        printf("Error: Bloque Lógico fuera de rango");
        return -1;
    }
}

int obtener_indice(int nblogico, int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico; //ej nblogico=8
    }
    else if (nblogico < INDIRECTOS0)
    {
        return (nblogico - DIRECTOS); //ej nblogico= 204
    }
    else if (nblogico < INDIRECTOS1)
    { //ej nblogico= 30.004
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2)
    { //ej nblogico= 400.004
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    //en caso de no entrar en nignún condicional, devolver error
    return -1;
}
//función que se encarga de obtener el numero de bloque físico correspondiente a un bloque lógico
//determinado del inodo indicado.
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar)
{

    struct inodo inodo;
    unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    int buffer[NPUNTEROS];

    leer_inodo(ninodo, &inodo);
    ptr = 0, ptr_ant = 0, salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                          //el nivel_punteros +alto es el que cuelga del inodo
    while (nivel_punteros > 0)
    {

        //iterar cada nivel de indirectos
        if (ptr == 0)
        {
            //no cuelgan blques de punteros
            if (reservar == 0)
            {
                printf("Error de lectura: bloque inexistente");
                return -1;
            }
            else
            {
                //reservar  bloques punteros y crear enlaces desde inodo hasta datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); //de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); //fecha actual
                if (nivel_punteros == nRangoBL)
                {
                    //el bloque cuelga directamente del inodo
                    printf("1[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nRangoBL - 1, ptr, ptr, nivel_punteros);
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; //(imprimirlo para test)
                }
                else
                {                         //el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr; //(imprimirlo para test)
                    printf("2[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
                    if (bwrite(ptr_ant, buffer) == -1)
                    {
                        fprintf(stderr, "Error de escritura");
                        return -1;
                    }
                }
            }
        }
        if (bread(ptr, buffer) == -1)
        {
            fprintf(stderr, "Error de lectura \n");
            return -1;
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        //guardamos el puntero
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    }
    //al salir de este bucle ya estamos al nivel de datos
    if (ptr == 0)
    { //no existe bloque de datos
        if (reservar == 0)
        {
            return -1; //error lectura bloque
        }
        else
        {

            salvar_inodo = 1;
            ptr = reservar_bloque(); //de datos
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if (nRangoBL == 0)
            {
                printf("3[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n", nblogico, ptr, ptr, nblogico);
                inodo.punterosDirectos[nblogico] = ptr; //imprimirlo para test
            }
            else
            {

                buffer[indice] = ptr; //imprimirlo para test
                printf("4[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n", indice, ptr, ptr, nblogico);
                if (bwrite(ptr_ant, buffer) == -1)
                {
                    fprintf(stderr, "Error de escritura \n");
                    return -1;
                }
            }
        }
    }
    if (salvar_inodo == 1)
    {
        //es posible que el error dado sea a causa del error anterior,
        //me abstengo de hacer ningún cambio, desde que adelaida lo tiene así
        //hasta que sea corregido el anterior y éste persista.
        if (escribir_inodo(inodo, ninodo) == -1)
        { //solo en caso de haber sido actualizado
            fprintf(stderr, " Error de ecritura en inodo \n");
            return -1;
        }
    }
    return ptr; //nbfísico del bloque de datos
}

//****  FUNCIONES DE NIVEL 6 ****
//libera el inodo que se pase por parámetro, requiere de una función auxiliar que libere todos los bloques lógicos
int liberar_inodo(unsigned int ninodo)
{

    struct inodo inodo;
    struct superbloque SB;

    //leemos el inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {

        fprintf(stderr, "Error lectura inodo en liberar_inodo de nivel 6 \n");
        return -1;
    }

    //llamar a la función auxiliar par aliberar todos los bloques del inodo
    int liberarBLS = liberar_bloques_inodo(0, &inodo); 

    //a la cantidad de bloques ocupados del inodo se le resta la cantidad de liberados de la función anterior
    inodo.numBloquesOcupados -= liberarBLS;

    //marcar el inodo como libre y tamEnBytesLog= 0
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    //actualizar lista enlazada de inodos libres:
    //leemos el superbloque para saber cuál es el primer inodo libre
    if (bread(posSB, &SB) == -1)
    {

        fprintf(stderr, "Error al leer superbloque liberar_inodo/nivel 6 \n");
        return -1;
    }
    //incluir el inodo que queremos liberar en la lista de inodos libres
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;

    //incluir el inodo que queremos liberar en la lista de inodos libres
    //el inodo liberado apuntará donde antes apuntaba el campo del SB
    SB.posPrimerInodoLibre = ninodo;

    //incrementar cantidad de inodos libres en el superbloque
    SB.cantBloquesLibres++;

    //escribir inodo
    if (escribir_inodo(inodo, ninodo) == -1)
    {

        fprintf(stderr, "Error al escribir inodo liberar_inodo/nivel 6 \n");
        return -1;
    }

    //escribir el superbloque
    if (bwrite(posSB, &SB) == -1)
    {

        fprintf(stderr, "Error al escribir superbloque liberar_inodo/nivel 6 \n");
        return -1;
    }

    //devuelve el nº del inodo liberado
    return ninodo;
}

//libera todos los bloques ocupados a partir del bloque lógico indicado por parámetro
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{

    unsigned int ultimoBL, nivel_punteros, indice, ptr, nBL;
    int nRangoBL;
    unsigned int bloque_punteros[NPUNTEROS];  //1024 bytes
    unsigned char bufAux_punteros[BLOCKSIZE]; //1024 bytes
    int ptr_nivel[3];
    int indices[3];
    int liberados;

    liberados = 0;
    //el fichero está vacío
    if ((inodo->tamEnBytesLog) == 0)
        return 0;
    //obtenemos el último bloque lógico del inodo
    if ((inodo->tamEnBytesLog % BLOCKSIZE) == 0)
    {

        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    }

    else
    {

        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
    //fijamos contenido del buffer auxiliar a 0
    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {

        nRangoBL = obtener_nRangoBL(*inodo, nBL, &ptr); //0:D, 1:IO,, 2:I0, 2I1, 3:I2
        if (nRangoBL < 0)
            return -1;
        nivel_punteros = nRangoBL; //el nivel_punteros +alto cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0)
        { //cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);
            if ((indice == 0) || (nBL = primerBL)){
            
            }
        }
    }
    //si la condición se cumple implica que el bloque de punteros está todo a 0
    // if(memcmp(bloque_punteros, bufAux_punteros, BLOCKSIZE) == 0){

    // }
    //wtf en el pseudocódigo de adelaida no aparece este último paso
    return liberados;

}