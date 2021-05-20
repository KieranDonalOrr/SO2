#include "directorios.h"

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
                *tipo = 'd';
                break;
            }
        }
        if (fichero)
        {
            strncpy(inicial, camino + 1, strlen(camino) - 1);
            strcpy(final, "");
            *tipo = 'f';
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
    bread(posSB, &SB);

    if (camino_parcial[0] == '/' && strlen(camino_parcial) == 1)
    {
        *p_inodo = SB.posInodoRaiz; //inodo raíz
        *p_entrada = 0;             //entrada 0 pertenece a inodo raíz
        return EXIT_SUCCESS;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1)
    {
        printf("ERROR_CAMINO_INCORRECTO\n");
        return -1;
    }

    //buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4)
    {
        printf("ERROR PERMISO DE LECTURA\n");
        return -1;
    }

    struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buf_entradas, 0, BLOCKSIZE / sizeof(struct entrada) * sizeof(struct entrada));
    //o bien un array de las entradas que caben en un bloque, para optimizar la lectura en RAM

     cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); //cantidad de entradas que contiene el inodo
     num_entrada_inodo = 0;                                                  //nº de entrada inicial
    if (cant_entradas_inodo > 0)
    {
        mi_read_f(*p_inodo_dir, buf_entradas, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE);
        while ((num_entrada_inodo < cant_entradas_inodo) && (inicial != entrada.nombre))
        {
            num_entrada_inodo++;
            mi_read_f(*p_inodo_dir, buf_entradas, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE);
        }
    }

    if (inicial != entrada.nombre)
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
                strcpy(entrada.nombre, inicial);
            if (tipo == 'd')
            {
                if (strcmp(final, "/") == 0)
                {
                    entrada.ninodo = reservar_inodo('d', 6);
                    fprintf(stderr, "[buscar_entrada()->reservado inodo: %d tipo 'd' con permisos %c para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                }
                else
                { //cuelgan más diretorios o ficheros
                    return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                }
            }
            else
            { //es un fichero
                entrada.ninodo = reservar_inodo('f', 6);
                printf("[buscar_entrada()->reservado inodo: %d tipo 'f' con permisos %c para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
            }

            if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) == -1)
            {
                fprintf(stderr, "[buscar_entrada()-> creada entrada: %s inodo: %d] \n", inicial, num_entrada_inodo);
                if (entrada.ninodo != -1)
                {
                    liberar_inodo(entrada.ninodo);
                    fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
                }
                return -1; //-1
            }
        }
    }

    if ((strcmp(final, "/") == 0) || tipo == 'f')
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar = 1))
        {
            //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos la recursividad
        *p_inodo = buf_entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].ninodo;
        *p_entrada = entrada.ninodo;
        return EXIT_SUCCESS;
    }
    else
    {
        *p_inodo_dir = buf_entradas[num_entrada_inodo].ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXIT_SUCCESS;
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
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
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
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    p_entrada = 0;
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
    return EXIT_SUCCESS;
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
    return 0;
}

//definida como variable global
static struct UltimaEntrada UltimaEntradaEscritura;

//lee el contenido de un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{

    unsigned int p_inodo, p_inodo_dir, p_entrada;
    int bytesEsc;

    //comprobamos si la escritura es sobre el mismo inodo
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {

        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    //sino llamar a buscar_entrada()
    else
    {

        buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 2);

        //actualizar los campos de UltimaEntradaEscritura con el p_inodo obtenido
        //con el camino buscado
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }

    bytesEsc = mi_write_f(p_inodo, buf, offset, nbytes);
    if (bytesEsc == -1)
    {

        fprintf(stderr, "Error de escritura, nivel 9 directorio.c");
        return -1;
    }

    //devuelve los bytes escritos
    return bytesEsc;
}

//variable global de mi_read
static struct UltimaEntrada UltimaEntradaLectura;

//lee los nbytes del fichero indicado por camino a partir del offest y los copia en el buffer.
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{

    unsigned int p_inodo;
    unsigned int p_inodo_dir, p_entrada;
    int bytesLeidos;

    //misma metodología que en mi_write, pero inversa
    //comprobamos la lectura sobre el mismo inodo
    if (strcmp(camino, UltimaEntradaLectura.camino) == 0)
    {

        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    else
    {
        //buscamos la entrada camino con buscar entrada para obtener el p_inodo
        buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 2); //no sé que hace el 2 este de permisos

        strcpy(UltimaEntradaLectura.camino, camino);
        UltimaEntradaLectura.p_inodo = p_inodo;
    }

    //si la entrada existe llamamos a mi_read_f
    bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    if (bytesLeidos == -1)
    {
        fprintf(stderr, "Fallo al leer directorio.c nivel9, mi_read");
        return -1;
    }

    //devuelve bytes leídos
    return bytesLeidos;
}
