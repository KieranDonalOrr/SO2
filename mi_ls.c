//Autores: Pablo Núñez Pérez, Kieran Donal Orr y Ander Sarrión Martín
#include "directorios.h"

int main(int argc, char **argv){
	
	char descriptor[1024];
	char buffer[10000];
	char tipo = '\0';
	
	//comprobación de sintaxis
	if(argc != 3){
		  fprintf(stderr, "Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
		return -1;
	}
	//Aqui miramomos si es directorio o fichero
	if((argv[2][strlen(argv[2])-1])=='/'){
	 	tipo ='d';
	}
	else{
	 	tipo ='f';
	}	
    strcpy(descriptor, argv[1]);
	bmount(descriptor);
	//muestra la mejora realizada en el mi_dir
	mi_dir(argv[2],buffer,&tipo);
	if((tipo =='d') && ((argv[2][strlen(argv[2])-1]) != '/')){
		fprintf(stderr,"Error de directorio\n");
		return -1;
	} 
	//Imprimimos el resultado
	fprintf(stderr,"Tipo\tModo      mtime\t\t\tTamaño\tNombre\n");
	fprintf(stderr,"-------------------------------------------------------------\n%s",buffer);

	bumount();

	return 0;
}

