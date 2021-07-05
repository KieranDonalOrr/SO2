//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "ficheros.h"
// Escribe el contenido de un buffer de memoria (buf_original), de tamao nbytes en un fichero/directorio.
// Le indicamos la posición de escritura inicial en bytes lógicos (offset) con respecto al inodo y
// el número de bytes que hay que escribir
int mi_write_f(unsigned int ninodo, const void *buf_original,unsigned int offset ,unsigned int nbytes){
    
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int bytes_escritos = 0;
    leer_inodo(ninodo, &inodo);

    if((inodo.permisos & 2) == 2){
        int primerBLogico = offset/BLOCKSIZE;
        int ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        mi_waitSem();
        int nbfisico = traducir_bloque_inodo(ninodo,primerBLogico,1);
        if(desp1 != 0){
            if(bread(nbfisico, buf_bloque)==-1){
                mi_signalSem();
                fprintf(stderr,"Error lectura en mi_write\n");
                return -1;
            }
        }
         if(nbfisico == -1){
             fprintf(stderr,"Error en la lectura del nbfisico");
             mi_signalSem();
             return -1;
            }
          mi_signalSem();
        if(bread(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error lectura en mi_write\n");
                return -1;
            }

        if(primerBLogico == ultimoBLogico){
    
            memcpy(buf_bloque + desp1, buf_original,nbytes);
            bytes_escritos += nbytes;
            if(bwrite(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error escritura en mi_write\n");
                return -1;
            }
        }
        else{
            desp2 = offset + nbytes - 1;
            memcpy (buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
            if(bwrite(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error escritura en mi_write\n");
                return -1;
            }
            bytes_escritos += BLOCKSIZE - desp1;

            for(int i = primerBLogico + 1; i != ultimoBLogico; i ++){
                mi_waitSem();
                nbfisico = traducir_bloque_inodo(ninodo, i, 1);
         if(nbfisico == -1){
             fprintf(stderr,"Error en la lectura del nbfisico");
             mi_signalSem();
             return -1;
            }
                bytes_escritos += bwrite(nbfisico, (buf_original + (BLOCKSIZE - desp1) + (i - primerBLogico - 1) * BLOCKSIZE));
                mi_signalSem();
            }
            
            desp2 = desp2 % BLOCKSIZE;
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 1);
         if(nbfisico == -1){
             fprintf(stderr,"Error en la lectura del nbfisico");
             mi_signalSem();
             return -1;
            }
         mi_signalSem();
            if(bread(nbfisico, buf_bloque)==-1){
               
                fprintf(stderr,"Error lectura en mi_write\n");
                return -1;
            }
            
            memcpy (buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
            bytes_escritos += desp2 + 1;
            if(bwrite(nbfisico, buf_bloque)==-1){
                fprintf(stderr,"Error escritura en mi_write\n");
                return -1;
            }
        }
        int bytes_escritos2 = bytes_escritos + offset;
        //volvemos a leer el inodo, se han reservado bloques
        mi_waitSem();
        if (leer_inodo(ninodo, &inodo) == -1)
        {
            mi_signalSem();
            fprintf(stderr, "Error lectura inodo\n");
            return -1;
        }
        if(inodo.tamEnBytesLog < bytes_escritos2){
            inodo.tamEnBytesLog = bytes_escritos2;
            inodo.ctime = time(NULL);
        }
        inodo.mtime = time (NULL);
        escribir_inodo(inodo, ninodo);
        mi_signalSem();
        return bytes_escritos; 
    }
    else{
        fprintf(stderr, "\nNo hay permisos de escritura\n");
        return -1;
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
        return -1; //no podemos leer nada
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

        mi_waitSem();
        //Actualizar inodos y atime
        if (leer_inodo(ninodo, &inodo) == -1)
        {

            fprintf(stderr, "Error de lectura del inodo\n");
            mi_signalSem();
            return -1;
        }
        inodo.atime = time(NULL);
        mi_signalSem();
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
    return 0;
}
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{

    struct inodo inodo;
    //lectura de inodo
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        fprintf(stderr, "Error lectura inodo\n");
        mi_signalSem();
        return -1;
    }
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    escribir_inodo(inodo, ninodo);
    mi_signalSem();
    return 0;
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