#include "ficheros_basico.h"
#include "bloques.h"
#include <time.h> 

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

    if ((resultTamAI % BLOCKSIZE) != 0)
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

    struct superbloque SB;
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

    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);

    struct superbloque SB;
    //contenido buffer se escribe en los bloques del mapa de bits
    for (int i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++)
    {
        bwrite(i, buffer);
    }
    return -1;
    //for()
    //escribir_bit(i,1);
}
//inicializar lista de inodos libres
int initAI()
{

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;
    //obtengo dirección array inodos
    bread(posSB, &SB);
    //se pone a 0 los inodos.
    memset(inodos, 0, BLOCKSIZE);

    SB.posPrimerBloqueAI = SB.posPrimerBloqueAI;
    SB.posUltimoBloqueAI = SB.posUltimoBloqueAI;
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    //si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
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
                //hay que salir del bucle, el último bloque no tiene por qué estar completo
                break;
            }
        }
        //escribir el bloque de inodos en el dispositivo virtual
        bwrite(i, &inodos);
    }
    return -1;
}
//escribe el valor indicado por el parametro bit (0 libre, 1 ocupado) en un determinado bloque
int escribir_bit(unsigned int nBloque, unsigned int bit)
{
    unsigned char bufferMB[BLOCKSIZE];
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
    memset(bufferMB, 0, BLOCKSIZE);
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
    return 0;
}
//lee determinado bit del MB y devuelve valor del bit leído
//misma operativa que en la función anterior
unsigned char leer_bit(unsigned int nBloque)
{
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;
    struct superbloque SB;
    int poSByte;
    int poSBit;
    int poSBlock;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    poSByte = nBloque / 8;
    poSBit = nBloque % 8;
    poSBlock = (poSByte / BLOCKSIZE) + SB.posPrimerBloqueMB;
    memset(bufferMB, 0, BLOCKSIZE);
    if (bread(poSBlock, bufferMB) == -1)
    {
        printf("Error en leer el bloque\n");
        return -1;
    }
    poSByte = poSByte % BLOCKSIZE;
    mascara >>= poSBit;
    mascara &= bufferMB[poSByte];
    mascara >>= (7 - poSBit);
    return mascara;
}
//encuentra primer bloque libre, lo ocupa y devuelve su posición
int reservar_bloque()
{
    struct superbloque SB;
    unsigned char buffer[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    int bloqueMB;
    int numBloque;
    int poSByte;
    int poSBit;
    unsigned char mascara = 128;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    //comprobamos si quedan bloques libres
    if (SB.cantBloquesLibres > 0)
    {
        //buffer auxiliar que utilizaremos para comparar cada bloque inicializado a 1
        memset(bufferAux, 255, BLOCKSIZE);
        //localizamos posicion primer bloque del MB
        bloqueMB = SB.posPrimerBloqueMB;
        if (bread(bloqueMB, buffer) == -1)
        {
            printf("Error en leer el mapa de bits\n");
            return -1;
        }
        //bucle que compara los bits de ambos buffer
        while (memcmp(bufferAux, buffer, BLOCKSIZE) == 0)
        {
            if (bloqueMB <= SB.posUltimoBloqueMB)
            {
                bloqueMB++;
                if (bread(bloqueMB, buffer) == -1)
                {
                    printf("Error error en leer el mapa de bits en bucle\n");
                    return -1;
                }
            }
            else
            {
                printf("Error no hay bloques libres\n");
                return -1;
            }
        }

        for (poSByte = 0; buffer[poSByte] == 255; poSByte++)
        {
        }
        poSBit = 0;
        if (buffer[poSByte] < 255)
        {
            for (; buffer[poSByte] & mascara; poSBit++)
            {
                buffer[poSByte] <<= 1;
            }
        }
        numBloque = ((bloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + poSByte) * 8 + poSBit;
        if (escribir_bit(numBloque, 1) != -1)
        {
            SB.cantBloquesLibres = SB.cantBloquesLibres - 1;
            if (bwrite(posSB, &SB) == -1)
            {
                printf("Error al escribir en el superbloque\n");
                return -1;
            }
            return numBloque;
        }
        else
        {
            printf("Error al escribir el bit\n");
            return -1;
        }
    }
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
struct inodo leer_inodo(unsigned int numInodo)
{
    struct superbloque SB;
    int nBloque;
    struct inodo ai[BLOCKSIZE / (BLOCKSIZE / 8)];
    //leemos superbloque para localizar array de inodos
    if (bread(posSB, &SB) == -1)
    {
        printf("Error al leer el superbloque\n");
        // No se qwue devolver aqui en caso de error
    }
    //inodo solicitado está en la siguiente posición
    nBloque = SB.posPrimerBloqueAI + (numInodo / (BLOCKSIZE / (BLOCKSIZE / 8)));
    if (bread(nBloque, ai) == -1)
    {
        printf("Error al leer el inodo\n");
         // No se que devolver aqui en caso de error
    }
    return ai[(numInodo % (BLOCKSIZE / (BLOCKSIZE / 8)))];
    //***según adelaida, ¿si funciona todo bien no ha de devolver 0?***
}


int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodo;
    int numInodo;

    if (bread(posSB, &SB) == -1)
    {
        printf("Error al leer el superbloque\n");
        return -1;
    }
    if (SB.cantInodosLibres > 0)
    {
        time_t now;
        inodo = leer_inodo(SB.posPrimerInodoLibre);
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
        int j;
        for (j = 0; i < 3; i++)
        {
            inodo.punterosIndirectos[i] = 0;
        }
        escribir_inodo(inodo, numInodo);
        SB.cantInodosLibres = SB.cantInodosLibres - 1;
        if (bwrite(posSB, &SB) == -1)
        {
            printf("Error en escribir en el superbloque\n");
            return -1;
        }
        return numInodo;
    }
    else
    {
        printf("Error no hay inodos libres\n");
        return -1;
    }
}
