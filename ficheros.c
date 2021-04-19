#include "ficheros_basico.h"
// Escribe el contenido de un buffer de memoria (buf_original), de tamao nbytes en un fichero/directorio.
// Le indicamos la posición de escritura inicial en bytes lógicos (offset) con respecto al inodo y
// el número de bytes que hay que escribir
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{

    struct inodo inodo;

    //lectura de inodo
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo\n");
        return -1;
    }
    //nos aseguramos de que haya permisos de escritura en el inodo
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "No hay permisos de escritura sobre el inodo\n");
        return -1;
    }
    //definicion del primer y último bloque lógico donde hay que escribir
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    //contador de numero de bytes escritos realmente (si todo va bien coincide con nbytes)
    int nbytesEscritosReal = 0;

    //calculamos desplazamiento en el bloque para el offset
    int desp1 = offset % BLOCKSIZE;
    //calculamos el desplazamiento en el bloque para ver hasta donde llegan los nbytes desde offset
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //primeramente leemos bloque físico del dispositivo virtual
    int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);

    //creamos array de caracteres de tamaño de un bloque
    char buf_bloque[BLOCKSIZE];

    //almacenamos bloque en buf_bloque
    if (bread(nbfisico, buf_bloque) == -1)
    {
        fprintf(stderr, "Error de lectura de bloque\n");
        return -1;
    }

    //tratamos el caso en que el primer y ultimo bloque coincidan, corresponderá a que hay un solo bloque
    if (primerBL == ultimoBL)
    {
        //escribimos los nbytes en la posición de buf_bloque+ desp1
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        //escribimos buf_bloque modificado en nbfisico
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
            fprintf(stderr, "Error de escritura en el bloque físico");
            return -1;
        }
        //como solo trata con un bloque:
        nbytesEscritosReal = nbytes;
    }
    //en caso de haber más de un bloque
    else
    {
        //distinguir las 3 fases:
        //Fase 1: Primer bloque lógico

        //copiar bytes del buf_original al buf_bloque
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        //hacemos bwrite con los nuevos datos, preservando los que contenía
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
            fprintf(stderr, "Error de escritura en el bloque físico");
            return -1;
        }

        //actualizamos conteo de bytes escritos, habiendo modificado (BLOCKSIZE - desp1)
        nbytesEscritosReal = nbytesEscritosReal + (BLOCKSIZE - desp1);

        //Fase 2: Bloques lógicos intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            //iteramos para cada bloque logico intermedio
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            //escribimos el fragmento correspondiente del buf_original
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1)
            {
                fprintf(stderr, "Error de escritura en el bloque físico\n");
                return -1;
            }

            //se escribe un bloque en cada iteración del for
            nbytesEscritosReal = nbytesEscritosReal + BLOCKSIZE;
        }

        //Fase 3: Último bloque lógico

        //hacemos bread del bloque fisico correspondiente, nbifisico
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (bread(nbfisico, buf_bloque) == -1)
        {
            fprintf(stderr, "Error de lectura en el bloque físico\n");
            return -1;
        }
        //calculamos desp2 y copiamos esos bytes a buf bloque
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        //hacemos bwrite de buf_bloque con los nuevos datos en la posicion de nbfisico
        if (bwrite(nbfisico, buf_bloque) == -1)
        {
            fprintf(stderr, "Error de escritura en el bloque físico correspondiente\n");
            return -1;
        }

        //actualizamos cantidad de bytes escritos
        nbytesEscritosReal = nbytesEscritosReal + (desp2 + 1);

        //Finalmente actualizaremos la metainformación del inodo
        //leemos inodo
        if (leer_inodo(ninodo, &inodo) == -1)
        {
            fprintf(stderr, "Error de lectura del inodo");
            return -1;
        }

        //actualizamos tamaño en bytes lógico si hemos escrito más allá del final del fichero
        //se dará cuando la suma de offset y nbytes sea mayor al del propio tamaño de bytes lógicos
        //y ctime si modificamos cualquier campo del inodo
        if (nbytes + offset > inodo.tamEnBytesLog)
        {
            inodo.tamEnBytesLog = nbytes + offset;
            time(inodo.ctime);
        }

        //actualizamos mtime por escribir en zona de datos
        time(inodo.mtime);

        //escribimos inodo
        if (escribir_inodo(inodo, ninodo) == -1)
        {
            fprintf(stderr, "Error de escritura del inodo\n");
            return -1;
        }
    }

    //devolvemos la cantidad de bytes escritos realmente
    return nbytesEscritosReal;
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
    int nbytesLeídosReal = 0;

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
        if(nbfisico != -1)
        {
            if(bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            //escribimos en nbytes (se ha invertido el memcpy de mi_write_f)
            memcpy(buf_original,buf_bloque + desp1, nbytes);

            
        }
        //como solo trata con un bloque:
        nbytesLeídosReal = nbytes;
        
    }
    //en caso de no haber un solo bloque
    else{

        //Fase 1: Primer bloque

    //leemos bloque físico del dispositivo virtual
    int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
     if(nbfisico != -1)
        {
            if(bread(nbfisico, buf_bloque) == -1)
            {
                fprintf(stderr, "Error de lectura en bloque físico\n");
                return -1;
            }
            //escribimos en nbytes (se ha invertido el memcpy de mi_write_f)
            memcpy(buf_original,buf_bloque + desp1, BLOCKSIZE - desp1);

        }
        //se lee después del if porque en caso de devolver -1 se debe contabilizar    
        nbytesLeídosReal = nbytesLeídosReal + (BLOCKSIZE - desp1);

        //Fase 2: Bloques lógicos intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            //iteramos para cada bloque logico intermedio
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            //escribimos el fragmento correspondiente del buf_original
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1)
            {
                fprintf(stderr, "Error de escritura en el bloque físico\n");
                return -1;
            }






    }

    //devolvemos cantidad de bytes leídos
    return nbytesLeídosReal;
}

//Mi stat, guarda los METADATOS de un nodo indicado por ninodo.
int mi_stat_f(unsigned int ninodo, struct stat *p_stat)
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
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{

    struct inodo nodo;
    //lectura de inodo
    if (leer_inodo(ninodo, &nodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo\n");
        return -1;
    }
    nodo.permisos = permisos;
    time(nodo.ctime);
}
