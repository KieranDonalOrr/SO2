#include "ficheros_basico.h"
#include "bloques.h"

//Devuelve el Tamaño en bloques del mapa de bits
int tamMB(unsigned int nbloques)
{

    //bits se agrupan en bloques de tamaño BLOCKSIZE
    int blocResult = (nbloques / 8);

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
int tamAI(unsigned int ninodos)
{

    //el sistema de ficheros utiliza ninodos = nbloques/4 <- mi_mkfs pasará el dato a esta función como parámetro

    int resultTamAI = ((ninodos * INODOSIZE) / BLOCKSIZE);

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
int initSB(unsigned int nbloques, unsigned int ninodos)
{

    struct superbloque SB;
    //primer bloque mapa bits
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB =1
    //ultimo bloque mapa bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    //primer bloque array inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    //último bloque array inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    //primer bloque datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    //ultimo bloque datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    //inodo directorio raíz
    SB.posInodoRaiz = 0;
    //primer inodo libre en array inodo
    SB.posPrimerInodoLibre = 0;
    //cantidad bloques libres en SF
    SB.cantBloquesLibres = nbloques;
    //cantidad inodos libres en array de inodos
    SB.cantInodosLibres = ninodos;
    //total de bloques
    SB.totBloques = nbloques;
    //total de inodos
    SB.totInodos = ninodos;

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

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;
    struct superbloque SB;
    int posByte;
    int posBit;
    int posBlock;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error in escribir_bit\n");
        return -1;
    }
    posByte = nbloque / 8;
    posBit = nbloque % 8;
    posBlock = (posByte / BLOCKSIZE) + SB.posPrimerBloqueMB;
    memset(bufferMB, 0, BLOCKSIZE);
    if (bread(posBlock, bufferMB) == -1)
    {
        printf("Error en leyendo el bloque en escribir bit\n");
        return -1;
    }
    mascara >>= posBit;
    posByte = posByte % BLOCKSIZE;
    if (bit == 1)
    {
        bufferMB[posByte] |= mascara;
    }
    else if (bit == 0)
    {
        bufferMB[posByte] &= ~mascara;
    }

    if (bwrite(posBlock, bufferMB) == -1)
    {
        printf("Error en escribir el bloque\n");
        return -1;
    }
    return 0;
}

unsigned char leer_bit(unsigned int nbloque)
{
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128;
    struct superbloque SB;
    int posByte;
    int posBit;
    int posBlock;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    posByte = nbloque / 8;
    posBit = nbloque % 8;
    posBlock = (posByte / BLOCKSIZE) + SB.posPrimerBloqueMB;
    memset(bufferMB, 0, BLOCKSIZE);
    if (bread(posBlock, bufferMB) == -1)
    {
        printf("Error en leer el bloque\n");
        return -1;
    }
    posByte = posByte % BLOCKSIZE;
    mascara >>= posBit;
    mascara &= bufferMB[posByte];
    mascara >>= (7 - posBit);
    return mascara;
}

int reservar_bloque()
{
    struct superbloque SB;
    unsigned char buffer[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    int bloqueMB;
    int numBloque;
    int posByte;
    int posBit;
    unsigned char mascara = 128;
    if (bread(posSB, &SB) == -1)
    {
        printf("Error en leer el superbloque\n");
        return -1;
    }
    if (SB.cantBloquesLibres > 0)
    {
        
        memset(bufferAux, 255, BLOCKSIZE);
        bloqueMB = SB.posPrimerBloqueMB;
        if (bread(bloqueMB, buffer) == -1)
        {
            printf("Error en leer el mapa de bits\n");
            return -1;
        }
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

        for (posByte = 0; buffer[posByte] == 255; posByte++)
        {
        }
        posBit = 0;
        if (buffer[posByte] < 255)
        {
            for (; buffer[posByte] & mascara; posBit++)
            {
                buffer[posByte] <<= 1;
            }
        }
        numBloque = ((bloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posByte) * 8 + posBit;
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
