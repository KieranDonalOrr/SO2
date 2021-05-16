#include "bloques.h"
#include "ficheros_basico.h"

void printSuperBloque(struct superbloque SB);
void printIA(struct superbloque SB);
void printBits(struct superbloque SB);

int main(int argc, char **argv)
{
	void *buf[BLOCKSIZE];
	struct superbloque SB;
	int descriptor;
	if (argc < 2)
	{
		printf("No hay suficentes argumentos");
		return -1;
	}
	descriptor = bmount(argv[1]);
	if (descriptor < 0)
	{
		exit(1);
	}
	if (bread(posSB, buf) < 0)
		return -1;
	
	memcpy(&SB, buf, sizeof(struct superbloque));
	
	printSuperBloque(SB);
	//printIA(SB);
	//printBits(SB);
	//int resver= reservar_inodo('f', 6);
	
	//traducir_bloque_inodo(resver, 8 , '1'); 
    //(traducir_bloque_inodo(resver, 204 , '1');
    //traducir_bloque_inodo(resver, 30004 , '1');
    //traducir_bloque_inodo(resver, 400004 , '1');
    //traducir_bloque_inodo(resver, 468750 , '1');


	if (bumount(descriptor) < 0)
	{
		exit(1);
	}
}

void printSuperBloque(struct superbloque SB)
{
	printf("DATOS DEL SUPERBLOQUE\n");
	printf("posPrimerBloqueMB : %d \n", SB.posPrimerBloqueMB);
	printf("posUltimoBloqueMB : %d \n", SB.posUltimoBloqueMB);
	printf("posPrimerBloqueAI  %d \n", SB.posPrimerBloqueAI);
	printf("posUltimoBloqueAI  %d \n", SB.posUltimoBloqueAI);
	printf("posPrimerBloqueDatos: %d \n", SB.posPrimerBloqueDatos);
	printf("posUltimaBloqueDatos: %d \n", SB.posUltimoBloqueDatos);
	printf("posInodoRaiz  %d \n", SB.posInodoRaiz);
	printf("posPrimerInodoLibre %d \n", SB.posPrimerInodoLibre);
	printf("cantBloquesLibres %d \n", SB.cantBloquesLibres);
	printf("cantInodos+Libres %d \n", SB.cantInodosLibres);
	printf("totBloques %d \n", SB.totBloques);
	printf("totInodos %d \n", SB.totInodos);
	printf("\n");
	printf("sizeof struct superbloque: %d \n", BLOCKSIZE);
	printf("sizeof struct inodo: %ld \n", sizeof(struct inodo));
}

void printBits(struct superbloque SB){
leer_bit(SB.posPrimerBloqueMB);
leer_bit(SB.posUltimoBloqueMB);
leer_bit(SB.posPrimerBloqueAI);
leer_bit(SB.posUltimoBloqueAI);
leer_bit(SB.posPrimerBloqueDatos);
leer_bit(SB.posUltimoBloqueDatos);
}

void printIA(struct superbloque SB)
{
	char atime[80], mtime[80], ctime[80];
	struct inodo inodo;
	
	struct tm *ts;
	printf("#Tamaño Inodo: %d\n", BLOCKSIZE / 8);
	for (int ninodo = 0; ninodo < SB.totInodos; ninodo++)
	{
		leer_inodo(ninodo, &inodo);
		
		if (inodo.tipo != 'l')
		{
			printf("%c",inodo.tipo);
			ts = localtime(&inodo.atime);
			strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
			ts = localtime(&inodo.mtime);
			strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
			ts = localtime(&inodo.ctime);
			strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
			printf("ID: %d ATIME: %s MTIME: %s CTIME: %s\n", ninodo, atime, mtime, ctime);
		}
		
	}
}