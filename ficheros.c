
#include "ficheros.h"
// Escribe el contenido de un buffer de memoria (buf_original), de tamao nbytes en un fichero/directorio.
// Le indicamos la posición de escritura inicial en bytes lógicos (offset) con respecto al inodo y
// el número de bytes que hay que escribir
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    void *buf_bloque = malloc(BLOCKSIZE);
    int total = 0;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    int primerBLogico = offset / BLOCKSIZE;
    int ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;
    leer_inodo(ninodo, &inodo);
    if ((inodo.permisos & 2) == 2)
    {
        int BFisico = traducir_bloque_inodo(ninodo, primerBLogico, 1);
        if (desp1 != 0)
        {
            bread(BFisico, buf_bloque);
        }
        if (primerBLogico == ultimoBLogico)
        {
            memcpy(buf_bloque + desp1, buf_original, desp2 + 1 - desp1);
            total += desp2 + 1 - desp1;
            bwrite(BFisico, buf_bloque);
        }
        else
        {
            desp2 = offset + nbytes - 1;
            memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
            total += BLOCKSIZE - desp1;
            bwrite(BFisico, buf_bloque);
            for (int i = primerBLogico + 1; i != ultimoBLogico; i++)
            {
                BFisico = traducir_bloque_inodo(ninodo, i, 1);
                total += bwrite(BFisico, (buf_original + (BLOCKSIZE - desp1) + (i - primerBLogico - 1) * BLOCKSIZE));
            }
            desp2 = desp2 % BLOCKSIZE;
            BFisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 1);
            bread(BFisico, buf_bloque);
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
            total += desp2 + 1;
            bwrite(BFisico, buf_bloque);
        }
        total += offset;
        leer_inodo(ninodo, &inodo);
        if (inodo.tamEnBytesLog < total)
        {
            inodo.tamEnBytesLog = total;
            inodo.ctime = time(NULL);
        }
        inodo.mtime = time(NULL);
        escribir_inodo(inodo, ninodo);
        return total;
    }
    else
    {
        fprintf(stderr, "\n¡No hay permisos de escritura!\n");

        return -1;
    }
}
//lee información de un fichero correspondiente a ninodo pasado como argumento, y lo almacena en buf_original.

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
   struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    void *buf_bloque = malloc(BLOCKSIZE);
    int offsetBuf = 0;
    if ((inodo.permisos & 4) == 4){
        int bytesLeidos = 0;
        if(offset >= inodo.tamEnBytesLog){
            bytesLeidos = 0;
            return bytesLeidos;
        }
        if((offset + nbytes) >= inodo.tamEnBytesLog){
            nbytes = inodo.tamEnBytesLog - offset;
        }
        int primerBLogico = offset/BLOCKSIZE;
        int ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        int BFisico = traducir_bloque_inodo(ninodo, primerBLogico, 0);
        if(primerBLogico == ultimoBLogico){
            if(BFisico != EXIT_FAILURE){
                bread(BFisico, buf_bloque);
                if(desp2 == 0){//caso en que se tenga que leer el ultimo bloque de una secuencia
                    memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
                    bytesLeidos += BLOCKSIZE - desp1;
                }else{
                    memcpy(buf_original, buf_bloque + desp1, desp2 + 1 - desp1);
                    bytesLeidos += desp2 + 1 - desp1;
                }
            }else{
                bytesLeidos += BLOCKSIZE;
            }
        }
        else{
            //posible falta de deteccion de bloques vacíos error
            desp2 = offset + nbytes - 1;
            if(BFisico != -1){
                //cosas con desp1!!!!
                bread(BFisico, buf_bloque);
                memcpy (buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
                bytesLeidos += BLOCKSIZE - desp1;
                offsetBuf += BLOCKSIZE - desp1;
            }else{
                bytesLeidos += BLOCKSIZE;
            }
            for(int i = primerBLogico + 1; i != ultimoBLogico; i ++){
                BFisico = traducir_bloque_inodo(ninodo, i, 0);
                if(BFisico != -1){
                    bytesLeidos += bread(BFisico, buf_bloque);
                    memcpy(buf_original + offsetBuf, buf_bloque, BLOCKSIZE);
                    offsetBuf += bread(BFisico, buf_bloque);
                }else{
                    bytesLeidos += BLOCKSIZE;
                }
            }
            desp2 = desp2 % BLOCKSIZE;
            BFisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 0);
            if(BFisico != -1){
                bread(BFisico, buf_bloque);
                memcpy (buf_original + offsetBuf, buf_bloque, desp2 + 1);
                bytesLeidos += desp2 + 1;
            }else{
                    bytesLeidos += BLOCKSIZE;
                }
        }
        inodo.atime = time(NULL);
        escribir_inodo(inodo, ninodo);
        return bytesLeidos;
       
    }else
    {   //no tienes permisos de lectura
        fprintf(stderr, "\n No hay permisos de lectura\n");
        return EXIT_FAILURE;
    }
    
}

//Mi stat, guarda los METADATOS de un nodo indicado por ninodo.
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo nodo;

    //lectura de inodo
    if (leer_inodo(ninodo, &nodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo\n");
        return -1;
    }

    //Tipo
    p_stat->tipo = nodo.tipo;
    //Permisos
    p_stat->permisos = nodo.permisos;
    //A time
    p_stat->atime = nodo.atime;
    //M time
    p_stat->mtime = nodo.mtime;
    //C time
    p_stat->ctime = nodo.ctime;
    //N links
    p_stat->nlinks = nodo.nlinks;
    //TamEnBytesLog
    p_stat->tamEnBytesLog = nodo.tamEnBytesLog;
    //NumBloqueOcupados
    p_stat->numBloquesOcupados = nodo.numBloquesOcupados;
    return 1;
}
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{

    struct inodo inodo;
    //lectura de inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo\n");
        return -1;
    }
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    return 1;
}
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    int primerBloqueLogico = 0;
    struct inodo inodoAux;
    int bloquesLiberados;

    leer_inodo(ninodo, &inodoAux);
    if ((inodoAux.permisos & 2) == 2)
    {
        if (nbytes >= inodoAux.tamEnBytesLog)
        {
            return -1;
        }
        if (nbytes % BLOCKSIZE == 0)
        {
            primerBloqueLogico = nbytes / BLOCKSIZE;
        }
        else
        {
            primerBloqueLogico = nbytes / BLOCKSIZE + 1;
        }
        bloquesLiberados = liberar_bloques_inodo(primerBloqueLogico, &inodoAux);
        inodoAux.numBloquesOcupados = inodoAux.numBloquesOcupados - bloquesLiberados;
        inodoAux.tamEnBytesLog = nbytes;
        inodoAux.mtime = time(NULL);
        inodoAux.ctime = time(NULL);
        escribir_inodo(inodoAux, ninodo);
        return bloquesLiberados;
    }
    return -1;
}