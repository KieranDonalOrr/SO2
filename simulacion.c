#include "simulacion.h"

//numero de procesos finalizados
static int acabados= 0;

int main(int argc, char **argv){


    pid_t pid;
    struct tm *ts;
    struct REGISTRO registro;
    time_t tiempo= time(NULL);
    //el valor del array de fecha es arbitrario
    //para los directorios, corresponde a los 100 procesos de prueba
    //siendo la mitad de los directorios hijos y la otra mitad padres
    char fecha[21], d[100], dp[50], dh[50];
    int error;
    //asignar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    //comprobar la sintaxis
    if(argc !=2){
        printf("uso: ./simulacion <disco>\n");
    }

    //montar el dispositivo
    bmount(argv[1]);

    //crear el directorio de simulación : /simul_aaaammddhhmmss/
    ts = localtime(&tiempo);
    //para la fecha, la he sacado de leer_sf.c, no tengo ni idea de si es así
    strftime(fecha, sizeof(fecha), "%a %Y-%m-%d %H:%M:%S", ts);
    memset(dp,0, sizeof(dp));
    strcpy(dp, "/simul_");
    strcat(dp, fecha);
    strcat(dp, "/");
    error = mi_creat(dp,7);
    //comprobación ejecución mi_creat
    if(error < 0 ){    
        fprintf(stderr, "Error al crear directorio (dp), simulacion.c/nivel12 \n");
        bumount();
        return -1;
    }

    for (int i = 1; i <= NUMPROCESOS; i++)
    {
        pid = fork();

        //si es el hijo entonces
        if(pid == 0){
            //montar dispositivo
            bmount(argv[1]);
            //crear el directorio de proceso añadiendo el PID al nombre 
            // osea, /simul_aaaammddhhmmss/proceso_pid
            memset(dh,0, sizeof(dh));

            sprintf(dh,"proceso_%d/", getpid()); //sprintf escribe el output en un string
            //añadimos todo al directorio total
            memset(d,0, sizeof(d)); //inicialiazmos d
            strcpy(d, dp); //ponemos dp
            strcat(d, dh); //añadimos dh

            error = mi_creat(d,7);
            //comprobación ejecución mi_creat
            if(error < 0 ){    
                fprintf(stderr, "Error al crear directorio (d), simulacion.c/nivel12 \n");
                bumount();
                return -1;
            }

            //crear fichero de prueba.dat
            //es decir /simul_aaaammddhhmmss/proceso_pid/prueba.dat
            stract(d,"prueba.dat");

            error = mi_creat(d,7);
            //comprobación ejecución mi_creat
            if(error < 0 ){    
                fprintf(stderr, "Error al crear fichero prueba.dat desde el directorio(d), simulacion.c/nivel12 \n");
                bumount();
                return -1;
            }

            //iniciar semilla de numeros aleatorios 
            srand(time(NULL) + getpid());  

            for(int nescritura= 1; nescritura<= NUMESCRITURAS; nescritura++){
                //inicializar el registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;

                error = mi_write(d,&registro, registro.nRegistro * sizeof(struct REGISTRO),sizeof (struct REGISTRO));
                if( error < 0){
                    fprintf(stderr, "Error de escritura simulacion.c/nivel12 \n");

                    bumount();
                    return -1;
                }
                usleep(50000); //50000 microsegundos = 0.05s

            }
            //desmontar dispositivo hijo
            bumount();
            exit(0);

        }

        usleep(200000); //20000 microsegundos = 0.2s

    }
    
    //Permitir que el padre espere por todos los hijos
    while(acabados < NUMPROCESOS){
        pause();
    }

    //desmontar dispositivo padre
    bumount();

    exit(0);


}

//funcion enterrador
void reaper(){
  pid_t ended;
  signal(SIGCHLD, reaper);
  while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
     acabados++;
  }
}
