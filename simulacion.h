#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>

#define NUMPROCESOS 100//Se han de generar 100 procesos
#define NUMESCRITURAS 50//Creara "prueba.dat" un total de 50 veces
#define REGMAX 500000 //permitiría un REGMAX = (((12+256+256²+256³)-1)*BLOCKSIZE)/sizeof(struct registro) pero 
                       //en la simulación lo limitaremos a 500.000 registros 

struct REGISTRO { //sizeof(struct REGISTRO): 24
   time_t fecha; //Precisión segundos
   pid_t pid; //PID del proceso que lo ha creado
   int nEscritura; //Entero con el número de escritura (de 1 a 50)
   int nRegistro; //Entero con el número del registro dentro del fichero
};