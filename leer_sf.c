#include "bloques.h"
#include "ficheros_basico.h"


void printSuperBloque(struct superbloque SB);

int main(int argc, char **argv){
	struct superbloque SB;
	int descriptor;
	if(argc<2){
		printf("No hay suficentes argumentos");
		return -1;
	}
	descriptor = bmount(argv[1]);
	if(descriptor<0){
		exit(1);
	}
	if(bread(posSB,&SB)<0) return -1;
	printSuperBloque(SB);

	if(bumount(descriptor)<0){
		exit(1);
	}
}

void printSuperBloque(struct superbloque SB){
	printf("DATOS DEL SUPERBLOQUE\n");
	printf("posPrimerBloqueMB : %d \n",SB.posPrimerBloqueMB);
	printf("posUltimoBloqueMB : %d \n",SB.posUltimoBloqueMB);
	printf("posPrimerBloqueAI  %d \n",SB.posPrimerBloqueAI);
	printf("posUltimoBloqueAI  %d \n",SB.posUltimoBloqueAI);
	printf("posPrimerBloqueDatos: %d \n",SB.posPrimerBloqueDatos);
	printf("posUltimaBloqueDatos: %d \n",SB.posUltimoBloqueDatos);
	printf("posInodoRaiz  %d \n",SB.posInodoRaiz);
	printf("posPrimerInodoLibre %d \n",SB.posPrimerInodoLibre);
	printf("cantBloquesLibres %d \n",SB.cantBloquesLibres);
	printf("cantInodos+Libres %d \n",SB.cantInodosLibres);
	printf("totBloques %d \n",SB.totBloques);
	printf("totInodos %d \n",SB.totInodos);
    printf("\n");
    printf("sizeof struct superbloque: %d \n",BLOCKSIZE);
	printf("sizeof struct inodo: %ld \n",sizeof(struct inodo));
}