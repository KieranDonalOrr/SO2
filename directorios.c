//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"
#define NIVEL10 0
struct superbloque SB;

//dada una cadena de caracteres camino (/), separa su contenido en dos
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    int fichero = 1;

    if (camino[0] == '/')
    {
        for (int i = 1; i < strlen(camino); i++)
        {
            if (camino[i] == '/')
            {
                strncpy(inicial, camino + 1, sizeof(char) * i - 1);
                fichero = 0;
                strcpy(final, camino + i);
                strcpy(tipo, "d");
                break;
            }
        }
        if (fichero)
        {
            strncpy(inicial, camino + 1, strlen(camino) - 1);
            strcpy(final, "");
            strcpy(tipo, "f");
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

//esta función busca una determinada entrada en tre las diferentes entradas del inodo correspondiente a us directorio padre.
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    memset(inicial, 0, sizeof(inicial));
    char final[strlen(camino_parcial) + 1];
    memset(final, 0, sizeof(final));
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    if (bread(posSB, &SB) == -1)
    { 
        fprintf(stderr, "Error de lectura en buscar_entrada/directorios.c/nivel7 \n");
        return -1;
    }
    if (camino_parcial[0] == '/' && strlen(camino_parcial) == 1)
    {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
#if NIVEL10
    fprintf(stderr, "[buscar_entrada() → inicial: %s, final: %s, reserva: %d] \n", inicial, final, reservar);
#endif
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    
    memset(entradas, 0, sizeof(entradas));
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;   
    if (cant_entradas_inodo > 0)
    {
        int offset = 0;
        offset += mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE);

        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) != 0))
        {

            num_entrada_inodo++;
            
            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {
                memset(entradas, 0, sizeof(entradas));
                offset += mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE);
            }
        }
        memcpy(&entrada, &entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))], sizeof(struct entrada));
    }

    
    if ((strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre)) != 0)
    {

        switch (reservar)
        {

        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            if ((inodo_dir.permisos & 2) != 2)
            {
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        entrada.ninodo = reservar_inodo('d', permisos);
#if NIVEL10
                        fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'd' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
#endif
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    entrada.ninodo = reservar_inodo('f', permisos);
#if NIVEL10
                    fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'f' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
#endif
                }

                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1)
                {

                    if (entrada.ninodo != -1)
                    {
#if NIVEL10
                        fprintf(stderr, "[buscar_entrada() → liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif

                        liberar_inodo(entrada.ninodo);
                    }

                    return -1;
                }
            }
        }
    }
    if (((strcmp(final, "/")) == 0) || ((strcmp(final, "")) == 0))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {

            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return 0;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;

        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return 0;
}

void mostrar_error_buscar_entrada(int error)
{
    
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    bread(posSB, &SB);
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);

    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}

int mi_dir(const char *camino, char *buffer, char *tipo)
{

    bread(0, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo, inicial = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &inicial, 0, '4');
    struct inodo inodos;

    char tmp[100];
    char tam[TAMFILA];
    //leemos y buscamos error
    if (error < 0)
    {
        return -1;
    }
    if (leer_inodo(p_inodo, &inodos) == -1)
    {
        fprintf(stderr, "Error al leer inodo en mi_dir/directorios.c/nivel8\n");
        return -1;
    }
    *tipo = inodos.tipo;
    if ((inodos.permisos & 2) != 2)
    {
        fprintf(stderr, "No tiene permisos de lectura \n");
        return -1;
    }
     if (inodos.tipo != 'd')
    {
        fprintf(stderr, "No es un directorio\n");
        return -1;
    }
    struct tm *tm;
    unsigned int numEntradas = inodos.tamEnBytesLog / sizeof(struct entrada);
    struct entrada Buffer_entradas[numEntradas];
    //volvemos a leer inodo
    error = leer_inodo(p_inodo, &inodos);
    if (error < 0)
    {
        fprintf(stderr, "Error al leer el inodo en mi_dir/directorios.c/nivel8\n");
        return -1;
    }
    //para cada entrada, leemos a cada inodo que está apuntando
    for (int i = 0; i < numEntradas; i++)
    {
        //buffer_entradas tiene tantas entradas como hijos visibles
        if (mi_read_f(p_inodo, &Buffer_entradas[i], i * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
            fprintf(stderr, "Error de lectura fichero en mi_dir/directorios.c/nivel8 \n");
            return -1;
        }
        if (leer_inodo(Buffer_entradas[i].ninodo, &inodos) < 0)
        {
            fprintf(stderr, "Error al leer el inodo en mi_dir/directorios.c/nivel8\n");
            return -1;
        }

        //mejora aplicada
        //lee si es directorio o fichero
        if (inodos.tipo == 'd')
        {
            //en caso de directorio, escribe una d
            strcat(buffer, "d");
        }
        else
        {
            //en caso de fichero, escribe una f
            strcat(buffer, "f");
        }
        strcat(buffer, "\t");
        
        //miramos los permisos de lectura
        //hace un & con 4, correspondiente al 3er bit (100), en caso de estar 1 tiene permisos 
        //de lectura
        if (inodos.permisos & 4)
        {
            strcat(buffer, "r");
        }
        else
        {
            strcat(buffer, "-");
        }
        //miramos los permisos de escritura
        //hace un & con 2, correspondiente al 2 bit (010), en caso de estar 1 se tiene permisos 
        //de escritura
        if (inodos.permisos & 2)
        {
            strcat(buffer, "w");
        }
        else
        {
            strcat(buffer, "-");
        }
        //miramos los permisos de ejecucion
        //hace un & con 1, correspondiente al 1 bit (001), en caso de estar 1 se tiene permisos 
        //de ejecucion
        if (inodos.permisos & 1)
        {
            strcat(buffer, "x");
        }
        else
        {
            strcat(buffer, "-");
        }
        strcat(buffer, "\t");
        //última modificación en el tiempo
        tm = localtime(&inodos.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        sprintf(tam, "\t%d\t", inodos.tamEnBytesLog);
        strcat(buffer, tam);
        //ponemos el nombre
        strcat(buffer, Buffer_entradas[i].nombre);
        strcat(buffer, "\n");
    }

    return 0;
}

int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir, p_inodo;
    unsigned int p_entrada = 0;
    int error;
    bread(posSB, &SB);
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    mi_chmod_f(p_inodo, permisos);
    return 0;
}

int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir, p_inodo;
    unsigned int p_entrada = 0;
    int error;
    bread(posSB, &SB);
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    mi_stat_f(p_inodo, p_stat);
    return p_inodo;
}
static struct UltimaEntrada UltimaEntradaEscritura;

//Función para escribir contenido en un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{

    int numBytesEscritos = 0;
    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    //Comprobamos que no estamos escribiendo sobre el mismo inodo que la ultima vez
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
     //En caso contrario volvemos a buscar el inodo usando buscar entrada
    else
    {
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
         //Actualizamos la ultima entrada escrita 
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    //Al tener el inodo escribimos 
    numBytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);
    return numBytesEscritos;
}

static struct UltimaEntrada UltimaEntradaLectura;
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{

    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int numBytesLeidos;

    //Comprobamos que no estamos leyendo sobre el mismo inodo que la ultima vez
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    //En caso contrario volvemos a buscar el inodo usando buscar entrada
    else
    {   
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        //Actualizamos la ultima entrada leida 
        strcpy(UltimaEntradaLectura.camino, camino);
        UltimaEntradaLectura.p_inodo = p_inodo;
    }
    //Al tener el inodo leemos 
    numBytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    return numBytesLeidos;
}

int mi_link(const char *camino1, const char *camino2)
{

    unsigned int p_inodo1, p_inodo2, p_inodo_dir, p_inodo_dir2, p_entrada, p_entrada2;
    struct inodo inodo1;
    int error = 0;
    struct entrada entrada;
    mi_waitSem();
    //necesariamente será un fichero.
    if ((camino1[strlen(camino1) - 1] == '/') && (camino2[strlen(camino2) - 1] == '/'))
    {
        fprintf(stderr, "Error: ambos caminos deben ser ficheros\n");
        mi_signalSem();
        return -1;
    }

    bread(posSB, &SB);

    p_inodo_dir = SB.posInodoRaiz;
    p_inodo_dir2 = SB.posInodoRaiz;
    p_inodo1 = SB.posInodoRaiz;
    p_inodo2 = SB.posInodoRaiz;

    //Leemos inodo
    error = buscar_entrada(camino1, &p_inodo_dir, &p_inodo1, &p_entrada, 0, 6);

    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }

    //en el caso que camino2 no exista, se crea mediante buscar_entrada con permisos 6
    //la llamamos en formato escritura para que devuelva error en caso de que ya exista
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }

    //leemos la entrada de camino 2
    error = mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));
    if (error < 0)
    {
        fprintf(stderr, "Error de lectura de la entrada camino2, nivel10 mi_link\n");
        mi_signalSem();
        return -1;
    }

    liberar_inodo(entrada.ninodo);

    //creamos el enlace asociado a la entrada de p_inodo1
    entrada.ninodo = p_inodo1;

    error = mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));
    if (error < 0)
    {
        fprintf(stderr, "Error de escritura de la entrada en p_inodo2\n");
    }

    //leemos el inodo2 para comprobar que se trate de un fichero
    error = leer_inodo(p_inodo1, &inodo1);
    if (error < 0)
    {
        fprintf(stderr, "Eror al leer inodo nivel10, mi_link\n");
        mi_signalSem();
        return -1;
    }

    //Incrementamos la cantidad de enlaces de p_inodo
    inodo1.nlinks++;
    inodo1.ctime = time(NULL);
    escribir_inodo(inodo1, p_inodo1);
    mi_signalSem();
    return 0;
}

//Funcion que borra la entrada de directorio especificada por el parametro camino
int mi_unlink(const char *camino)
{

    //Struct Entrada
    struct entrada input;

    //Punteros a los inodos
    unsigned int puntero_directorio = 0;
    unsigned int puntero_inodo;
    unsigned int puntero_entrada;
    //Inodo
    struct inodo inodo;
    mi_waitSem();
    bread(posSB, &SB);
    puntero_directorio = puntero_inodo = SB.posInodoRaiz;

    //Check si existe la entrada
    int resultado = buscar_entrada(camino, &puntero_directorio, &puntero_inodo, &puntero_entrada, 0, 6);
    if (resultado < 0)
    {
        mostrar_error_buscar_entrada(resultado);
        mi_signalSem();
        return -1;
    }

    // Caso directorio no vacio
    if (leer_inodo(puntero_inodo, &inodo) == -1)
    {
        mi_signalSem();
        return -1;
    }
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0)
    {
        mi_signalSem();
        return -1;
    }

    // Leemos inodo que contiene la entrada
    if (leer_inodo(puntero_directorio, &inodo) == -1)
    {
        mi_signalSem();
        return -1;
    }

    int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    if (puntero_entrada != numEntradas - 1)
    {
        // Read ultima entrada
        mi_read_f(puntero_directorio, &input, (numEntradas - 1) * sizeof(struct entrada), sizeof(struct entrada));

        // Write en la posición de la entrada a eliminar
        mi_write_f(puntero_directorio, &input, puntero_entrada * sizeof(struct entrada), sizeof(struct entrada));
    }

    mi_truncar_f(puntero_directorio, (numEntradas - 1) * sizeof(struct entrada));

    // Leemos el inodo asociado a la entrada borrada
    if (leer_inodo(puntero_inodo, &inodo) == 1)
    {
        mi_signalSem();
        return -1;
    }

    // Actualizamos enlaces del inodo...
    inodo.nlinks--;
    if (inodo.nlinks == 0)
    {
        liberar_inodo(puntero_inodo);
    }
    else
    {
        inodo.ctime = time(NULL);
        escribir_inodo(inodo, puntero_inodo);
    }
    mi_signalSem();
    return 0;
}