
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

        return EXIT_FAILURE;
    }
}
//lee información de un fichero correspondiente a ninodo pasado como argumento, y lo almacena en buf_original.

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    //Inodo
    struct inodo inodo;

    //Lectura de inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo\n");
        return -1;
    }
    //Nos aseguramos de que haya permisos de lectura en el inodo
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "No hay permisos de lectura sobre el inodo\n");
        return -1;
    }

    //la función no puede leerse más allá del tamao en bytes lógicos del inodo
    if (offset >= inodo.tamEnBytesLog)
    {
        return 0; //no podemos leer nada
    }
    //si se pretende leer más allá del EOF
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        //leemos solo los bytes que podemos desde el offset hasta el EOF
        nbytes = inodo.tamEnBytesLog - offset;
    }
    //planteamos mismos casos que en mi_write_f():

    //definicion del primer y último bloque lógico donde hay que escribir
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    //contador de numero de bytes escritos realmente (si todo va bien coincide con nbytes)
    int nbytesLeidosReal = 0;

    //calculamos desplazamiento en el bloque para el offset
    int desp1 = offset % BLOCKSIZE;
    //calculamos el desplazamiento en el bloque para ver hasta donde llegan los nbytes desde offset
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //creamos array de caracteres de tamaño de un bloque
    char buf_bloque[BLOCKSIZE];

    //tratamos el caso en que el primer y ultimo bloque coincidan, corresponderá a que hay un solo bloque
    if (primerBL == ultimoBL)
    {
        //primeramente leemos bloque físico del dispositivo virtual
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != -1)
        {
            if (bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            //escribimos en nbytes (se ha invertido el memcpy de mi_write_f)
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        //como solo trata con un bloque:
        nbytesLeidosReal = nbytes;
    }
    //en caso de no haber un solo bloque
    else
    {

        //Fase 1: Primer bloque

        //leemos bloque físico del dispositivo virtual
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != -1)
        {
            if (bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            //escribimos en nbytes (se ha invertido el memcpy de mi_write_f)
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        //se lee después del if porque en caso de devolver -1 se debe contabilizar
        nbytesLeidosReal = nbytesLeidosReal + (BLOCKSIZE - desp1);

        //Fase 2: Bloques lógicos intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            //iteramos para cada bloque logico intermedio
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != -1)
            {
                if (bread(nbfisico, buf_bloque) == -1)
                {
                    fprintf(stderr, "Error de lectura en bloque físico\n");
                    return -1;
                }

                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            nbytesLeidosReal = nbytesLeidosReal + BLOCKSIZE;
        }

        //Fase 3: Bloque final
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != -1)
        {
            if (bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            //escribimos en nbytes (se ha invertido el memcpy de mi_write_f)
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }
        nbytesLeidosReal = nbytesLeidosReal + (desp2 + 1);

        //Actualizar inodos y atime
        if (leer_inodo(ninodo, &inodo) == -1)
        {

            fprintf(stderr, "Error de lectura del inodo\n");
            return -1;
        }
        inodo.atime = time(NULL);
    }

    //devolvemos cantidad de bytes leídos
    return nbytesLeidosReal;
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