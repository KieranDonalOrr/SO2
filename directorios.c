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

    int cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); //cantidad de entradas que contiene el inodo
    int num_entrada_inodo = 0;                                                  //nº de entrada inicial
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
            if (inodo_dir.tipo = 'f')
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
            if (tipo = 'd')
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
                printf(stderr, "[buscar_entrada()->reservado inodo: %d tipo 'f' con permisos %c para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
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
