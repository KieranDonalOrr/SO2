#include "directorios.h"
#define DEBUG 0
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

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB;
    struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];
    int error = 0;
    int creado = 0;
    bread(posSB, &SB);
    memset(&inicial, 0, sizeof(entrada.nombre));
    memset(&final, 0, strlen(camino_parcial));

    if (camino_parcial[0] == '/' && strlen(camino_parcial) == 1)
    {
        *p_inodo = SB.posInodoRaiz; //inodo raíz
        *p_entrada = 0;             //entrada 0 pertenece a inodo raíz
        return 0;
    }
    error = extraer_camino(camino_parcial, inicial, final, &tipo);

    if (error == -1)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
    #if DEBUG
    fprintf(stderr, "[buscar_entrada()-> inicial: %s, final: %s, reserva: %d] \n", inicial, final, reservar);
    #endif
    //buscamos la entrada cuyo nombre se encuentra en inicial
    
    if (leer_inodo(*p_inodo_dir, &inodo_dir)==-1)
    {
        return ERROR_PERMISO_LECTURA;
    }
    
    //o bien un array de las entradas que caben en un bloque, para optimizar la lectura en RAM
    num_entrada_inodo = 0;                                                  //nº de entrada inicial
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); //cantidad de entradas que contiene el inodo
    int offset = 0;
    memset(buf_entradas, 0, BLOCKSIZE / sizeof(struct entrada) * sizeof(struct entrada));
    if (cant_entradas_inodo > 0)
    {
        offset = mi_read_f(*p_inodo_dir, buf_entradas, offset, BLOCKSIZE);
        while ((num_entrada_inodo < cant_entradas_inodo) &&  (strcmp(inicial, buf_entradas[num_entrada_inodo].nombre) != 0))
        {
            num_entrada_inodo++;

            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {

                offset += mi_read_f(*p_inodo_dir, buf_entradas, offset, BLOCKSIZE);
            }
        }
    }

    if ((num_entrada_inodo == cant_entradas_inodo) && (inicial != buf_entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre))
    { //la entrada no existe
        switch (reservar)
        {
        case 0: //modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: //modo escritura
            //Creamos la entrada en el directorio referenciado por *p_inodo_dir
            //si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            //si es directorio comprobar que tiene permiso de escritura
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
                        entrada.ninodo = reservar_inodo('d', 6);
                        #if DEBUG
                        fprintf(stderr, "[buscar_entrada()->reservado inodo: %d tipo 'd' con permisos %c para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                         #endif
                    }
                    else
                    { //cuelgan más diretorios o ficheros
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                { //es un fichero
                    entrada.ninodo = reservar_inodo('f', 6);
                    #if DEBUG
                    printf("[buscar_entrada()->reservado inodo: %d tipo 'f' con permisos %c para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                    #endif
                }

                error = mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada));
                    #if DEBUG
                fprintf(stderr, "[buscar_entrada()-> creada entrada: %s %d] \n", inicial, num_entrada_inodo);
                    #endif
                if (error == -1)
                {

                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
                            #if DEBUG
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
                        #endif
                    }
                    return -1; //-1
                }
                creado = 1;
            }
        }
    }

    if ((strcmp(final, "/") == 0) || tipo == 'f')
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos la recursividad
        if (!creado)
        {
            *p_inodo = buf_entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].ninodo;
        }
        else
        {
            *p_entrada = entrada.ninodo;
        }
        *p_entrada = num_entrada_inodo;
        return 0;
    }
    else
    {
        *p_inodo_dir = buf_entradas[num_entrada_inodo].ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
}

void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
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
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    bread(posSB, &SB);
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);

    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    return 0;
}

int mi_dir(const char *camino, char *buffer)
{
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    p_entrada = 0;
     bread(posSB, &SB);
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);

    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    int offset = 0;
    struct entrada entrada;
    struct inodo inodo;
    if (camino[strlen(camino) - 1] == '/')
    { //mi_read_f(p_inodo, &entrada, offset, sizeof(struct entrada));
        leer_inodo(p_inodo, &inodo);
        int cant_entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
        struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];
        offset = offset + mi_read_f(p_inodo, buf_entradas, offset, BLOCKSIZE);

        for (int i = 0; i < cant_entradas_inodo; i++)
        {

            leer_inodo(buf_entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo);

            strcat(buffer, buf_entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre); //ponemos el nombre en el buffer

            if ((strlen(buffer) % 100) != 0)
            { //rellenamos hasta 100
                while ((strlen(buffer) % 100) != 0)
                {
                    strcat(buffer, " ");
                }
            }
            strcat(buffer, "\n");
            if ((offset % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            {
                offset += mi_read_f(p_inodo, buf_entradas, offset, sizeof(struct entrada));
            }
        }
    }
    else
    {
        mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada));
        leer_inodo(entrada.ninodo, &inodo);

        strcat(buffer, entrada.nombre);

        if ((strlen(buffer) % 100) != 0)
        {
            while ((strlen(buffer) % 100) != 0)
            {
                strcat(buffer, " ");
            }
        }
        strcat(buffer, "\n");
    }
    return 0;
}

int mi_chmod(const char *camino, unsigned char permisos)
{
    struct superbloque SB;
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
    struct superbloque SB;
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

//definida como variable global
static struct UltimaEntrada UltimaEntradaEscritura;

//lee el contenido de un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    struct superbloque SB;
    unsigned int p_inodo, p_inodo_dir, p_entrada;
    int bytesEsc;
    bread(posSB, &SB);
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    int error = 0;

    //comprobamos si la escritura es sobre el mismo inodo
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {

        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    //sino llamar a buscar_entrada()
    else
    {

        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 2);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }

        //actualizar los campos de UltimaEntradaEscritura con el p_inodo obtenido
        //con el camino buscado
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }

    bytesEsc = mi_write_f(p_inodo, buf, offset, nbytes);
    //devuelve los bytes escritos
    return bytesEsc-offset;
}

//variable global de mi_read
static struct UltimaEntrada UltimaEntradaLectura;

//lee los nbytes del fichero indicado por camino a partir del offest y los copia en el buffer.
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    struct superbloque SB;
    unsigned int p_inodo;
    unsigned int p_inodo_dir, p_entrada;
    int bytesLeidos;
    bread(posSB, &SB);
    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    int error = 0;

    //misma metodología que en mi_write, pero inversa
    //comprobamos la lectura sobre el mismo inodo
    if (strcmp(camino, UltimaEntradaLectura.camino) == 0)
    {

        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    else
    {
        //buscamos la entrada camino con buscar entrada para obtener el p_inodo
        error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);

        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
            return -1;
        }
        strcpy(UltimaEntradaLectura.camino, camino);
        UltimaEntradaLectura.p_inodo = p_inodo;
    }

    //si la entrada existe llamamos a mi_read_f
    bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    if (bytesLeidos == -1)
    {
        fprintf(stderr, "Fallo al leer directorio.c nivel9, mi_read\n");
        return -1;
    }

    //devuelve bytes leídos
    return bytesLeidos;
}

//crea el enlace de entrada de directorio camino2 al inodo especificado por otra entrada de camino1
int mi_link(const char *camino1, const char *camino2)
{

    struct superbloque SB;
    unsigned int p_inodo1, p_inodo2, p_inodo_dir, p_inodo_dir2, p_entrada, p_entrada2;
    struct inodo inodo1;
    int error = 0;
    struct entrada entrada;

    //necesariamente será un fichero.
    if ((camino1[strlen(camino1) - 1] == '/') && (camino2[strlen(camino2) - 1] == '/'))
    {
        fprintf(stderr, "Error: ambos caminos deben ser ficheros\n");
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
        return -1;
    }

    //en el caso que camino2 no exista, se crea mediante buscar_entrada con permisos 6
    //la llamamos en formato escritura para que devuelva error en caso de que ya exista
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    //leemos la entrada de camino 2
    error = mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));
    if (error < 0)
    {
        fprintf(stderr, "Error de lectura de la entrada camino2, nivel10 mi_link\n");
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
        return -1;
    }

    //Incrementamos la cantidad de enlaces de p_inodo
    inodo1.nlinks++;
    inodo1.ctime = time(NULL);
    escribir_inodo(inodo1, p_inodo1);
    return 0;
}

//Funcion que borra la entrada de directorio especificada por el parametro camino
int mi_unlink(const char *camino)
{
    struct superbloque SB;
    //Struct Entrada
    struct entrada input;

    //Punteros a los inodos
    unsigned int puntero_directorio = 0;
    unsigned int puntero_inodo;
    unsigned int puntero_entrada;
    //Inodo
    struct inodo inodo;

    bread(posSB, &SB);
    puntero_directorio = puntero_inodo = SB.posInodoRaiz;

    //Check si existe la entrada
    int resultado = buscar_entrada(camino, &puntero_directorio, &puntero_inodo, &puntero_entrada, 0, 6);
    if (resultado < 0)
    {
        mostrar_error_buscar_entrada(resultado);
        return -1;
    }

    // Caso directorio no vacio
    if (leer_inodo(puntero_inodo, &inodo) == -1)
    {
        return -1;
    }
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0)
    {
        return -1;
    }

    // Leemos inodo que contiene la entrada
    if (leer_inodo(puntero_directorio, &inodo) == -1)
    {
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

    return 0;
}
