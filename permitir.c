#include "ficheros.h"

int permitir(int argc, char **argv)
{
    //validación de sintaxis
    if( argc !=4){

        fprintf(stderr, "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>");
        return -1;
    }
    //montamos dispositivo
    bomunt(argv[1]);

    //llamada a mi_chmod_f() con los argumentos recibidos convertidos a enteros

    unsigned int permisos = atoi(argv[3]);
    unsigned int ninodo = atoi(argv[2]);
    
    //testeo errores para saber si falla
    if(mi_chmod_f(ninodo, permisos) == -1)
    {
        fprintf(stderr, "Error de ejecución de mi_chmod\n");
        return -1;

    }

    //desmonto dispositivo
    bumount();

    return 0;
}