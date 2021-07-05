//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "simulacion.h"
#define DEBUG13 1
int acabados = 0;
void reaper(){
  pid_t ended;
  signal(SIGCHLD, reaper);
  while ((ended = waitpid(-1, NULL, WNOHANG)) > 0){
    acabados++;
  }
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "Error de sintaxis: ./simulacion <disco>\n" );
        return -1;
    }
    char descriptor[1024];
    strcpy(descriptor,argv[1]);
    bmount(descriptor);

    char camino[21] = "/simul_";
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    sprintf(camino + strlen(camino), "%d%02d%02d%02d%02d%02d/",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    mi_creat(camino, 6);
    signal(SIGCHLD, reaper);

    pid_t pid;
    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++)
    {
        pid = fork();

        if (pid == 0)
        {
            if (bmount(descriptor) == -1)
            {
                fprintf(stderr, "./simulacion: Error al  montar el dispositivo\n");
                 exit(0);
            }
            char camino2[40];
            memset(camino2,0,sizeof(camino2));
            sprintf(camino2, "%sproceso_%d/", camino, getpid());
            mi_creat(camino2, 6);   
            char camino3[50];
            memset(camino3,0,sizeof(camino3));
            sprintf(camino3, "%sprueba.dat", camino2);
            mi_creat(camino3, 6);


            srand(time(NULL) + getpid());
            for (int nescritura = 0; nescritura < NUMESCRITURAS; nescritura++)
            {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura + 1;
                registro.nRegistro = rand() % REGMAX;
                mi_write(camino3, &registro, (registro.nRegistro * sizeof(struct REGISTRO)), sizeof(struct REGISTRO));

                usleep(50000); 
            }
#if DEBUG13
            fprintf(stderr, "[Proceso %d: Completadas %d escrituras en %s]\n", proceso, NUMESCRITURAS, camino3);
#endif
            if (bumount() == -1)
            {
                fprintf(stderr, "./simulación.c: Error desmontando el dispositivo\n");
                exit(0);
            }
            exit(0);
        }
        usleep(200000); 
    }
    
    while (acabados < NUMPROCESOS)
    {
        pause();
    }
    bumount();
    return 0;
}