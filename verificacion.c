//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "verificacion.h"
#define NIVEL13 1
int main(int argc, char **argv){

    // Comprobar la sintaxis  Uso: verificacion <nombre_dispositivo> <directorio_simulación>
    if(argc!=3){
        fprintf(stderr,"La sintaxis es: verificacion <nombre_dispositivo><directorio_simulación>");
        return -1;
    }

    //Montar el dispositivo
    char descriptor[1024];
    strcpy(descriptor, argv[1]);  
    bmount(descriptor);
    struct STAT stat;
    mi_stat(argv[2],&stat);
    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo
    int numentradas = stat.tamEnBytesLog/sizeof(struct entrada);
    //devolvemos error en caso de que numero de entradas sea diferente al numero de procesos
    if(numentradas != NUMPROCESOS){
        bumount();
        return -1;
    }
#if NIVEL13
    fprintf(stderr, "numentradas: %i, NUMPROCESOS: %i\n", numentradas, NUMPROCESOS);
#endif

    // Crear el fichero "informe.txt" dentro del directorio de simulación
    char dir[5000];
    strcpy(dir,argv[2]);
    strcat(dir,"informe.txt");
    if((mi_creat(dir,7))==-1){

        if(bumount() == -1){
            fprintf(stderr,"Error al desmontar el dispositivo\n");
        }
        fprintf(stderr,"Error mi_creat/verificacion\n");
        return -1;
    }
    //Leer los directorios correspondientes a los procesos 
    struct entrada buffEntrada[numentradas];    //Entradas del directorio de simulación
    mi_read(argv[2],&buffEntrada,0,sizeof(buffEntrada));

    
     
    int num_bytes = 0;
    //Para cada entrada directorio de un proceso hacer
    for(int i = 0; i < numentradas; i++){

                
        //Mejora propuesta por Adelaida:
        //utilizamos strchr con '_' y mediante atoi pasamos el valor del PiD a entero.
        //Leemos la entrada del directorio
        pid_t pid = atoi(strchr(buffEntrada[i].nombre, '_') + 1);
        struct INFORMACION info;
        info.pid = pid;
        info.nEscrituras = 0;
        //Recorrer secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras:  
        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        char prueba[BLOCKSIZE]; 
        //creación directorio prueba.dat, donde leerá posteriormente para no tener que llamar a
        //buscar_entrada para leer cada escritura en un mismo proceso.
        sprintf(prueba, "%s%s/%s", argv[2], buffEntrada[i].nombre, "prueba.dat"); 
        int offset = 0;
        //mientras haya escrituras en prueba.dat:
        while (mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0){
    

           //leemos una escritura
            int nregistro = 0;
            while (nregistro < cant_registros_buffer_escrituras)
            {
                //comprobamos su validez verificando el pid de la escritura, pues debe coincidir con el del proceso
                if (buffer_escrituras[nregistro].pid == info.pid)
                {
                    //Si es la primera escritura validada entonces
                    if (!info.nEscrituras)
                    {
                        //inicializar los registros significativos con los datos de esa escritura
                        info.MenorPosicion = buffer_escrituras[nregistro];
                        info.MayorPosicion = buffer_escrituras[nregistro];
                        info.PrimeraEscritura = buffer_escrituras[nregistro];
                        info.UltimaEscritura = buffer_escrituras[nregistro];
                        info.nEscrituras++;
                    }
                    else{
                        //Comparar nº de escritura (para obtener primera y última) y actualizarla si es preciso
                        if ((difftime(buffer_escrituras[nregistro].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[nregistro].nEscritura < info.PrimeraEscritura.nEscritura){
                            info.PrimeraEscritura = buffer_escrituras[nregistro];
                        }
                        if ((difftime(buffer_escrituras[nregistro].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[nregistro].nEscritura > info.UltimaEscritura.nEscritura){
                            info.UltimaEscritura = buffer_escrituras[nregistro];
                        }
                        if (buffer_escrituras[nregistro].nRegistro < info.MenorPosicion.nRegistro){
                            info.MenorPosicion = buffer_escrituras[nregistro];
                        }
                        if (buffer_escrituras[nregistro].nRegistro > info.MayorPosicion.nRegistro){
                            info.MayorPosicion = buffer_escrituras[nregistro];
                        }
                        //Incrementar contador escrituras validadas
                        info.nEscrituras++;
                    }
                }
                nregistro++;
            }
            //obtener escritura de la última posición
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);
        }
#if NIVEL13
        fprintf(stderr, "[%i) %i escrituras validadas en %s]\n", i + 1, info.nEscrituras, prueba);
#endif
        //Añadir la información del struct info al fichero informe.txt por el final
        char tiempoPrimero[100];
        char tiempoUltimo[100];
        char tiempoMenor[100];
        char tiempoMayor[100];
        struct tm *tm;

        tm = localtime(&info.PrimeraEscritura.fecha);
        strftime(tiempoPrimero, sizeof(tiempoPrimero), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.UltimaEscritura.fecha);
        strftime(tiempoUltimo, sizeof(tiempoUltimo), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MenorPosicion.fecha);
        strftime(tiempoMenor, sizeof(tiempoMenor), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MayorPosicion.fecha);
        strftime(tiempoMayor, sizeof(tiempoMayor), "%a %Y-%m-%d %H:%M:%S", tm);

        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);
        //Metemos toda la informacion a immprimir en un buffer y lo 
        sprintf(buffer, "PID: %i\nNumero de escrituras: %i\n", pid, info.nEscrituras);
        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Primera escritura",
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                asctime(localtime(&info.PrimeraEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Ultima escritura",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                asctime(localtime(&info.UltimaEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Menor posicion",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                asctime(localtime(&info.MenorPosicion.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Mayor posicion",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                asctime(localtime(&info.MayorPosicion.fecha)));

        sprintf(buffer,
                "PID: %d\nNumero de escrituras:\t%d\nPrimera escritura:"
                "\t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMayor po"
                "sición:\t\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                tiempoPrimero,
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                tiempoUltimo,
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                tiempoMenor,
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                tiempoMayor);

        //escribimos en el fichero junto al offsetw
        if ((num_bytes += mi_write(dir, &buffer, num_bytes, strlen(buffer))) < 0){
            printf("Error de escritura en fichero mi_write/verificacion.c/nivel13: '%s'\n", dir);
            bumount();
            return -1;
        }  
    }
    
    //Desmontar el dispositivo  
    bumount();
}
