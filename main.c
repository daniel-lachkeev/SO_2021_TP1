#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "input.h"

#define TEST_LINES 5
#define TEST_PATH "cpu_tests/"

typedef struct problema {
	int n;
	int m;
	int maxComprimento;
	int* compPecas;
	int* qttdPecas;
} Problema;

Problema getProblem(char** lines);

Problema loadTest(char* filename, int p) {
	char filepath[100] = "";
	strcat(filepath, "cpu_tests/");
	strcat(filepath, filename);
	
	char** file = (char**)malloc(TEST_LINES * sizeof(char*));

	FILE* stream = fopen(filepath, "r");

	int count = 0;

	printf("%d processos a usar...\n", p);

	char line[1024];
	while (fgets(line, 1024, stream)) {
		if(count == TEST_LINES) break;

		char* tmp = strdup(line);
		file[count] = (char*)malloc(sizeof(tmp)/sizeof(char));
		strcpy(file[count], tmp);

		printf("%s", tmp);


		free(tmp);
		count++;
	}
	printf("\n");
	getProblem(file);

	fclose(stream);
}

Problema getProblem(char** lines) {
	int n = atoi(lines[0]);
	int m = atoi(lines[1]);
	int maxComprimento = atoi(lines[2]);

	char** tokens1 = splitString(lines[3], n, " ");
	char** tokens2 = splitString(lines[4], n, " ");

	int compPecas[n];
	int qtddPecas[n];
/*
	printf("n:%d\n", n);
	printf("m:%d\n", m);
	printf("maxC:%d\n", maxComprimento);*/

	for(int i = 0; i < n; i++) {
		/*
		printf("c%d: %s\n", i, tokens1[i]);
		printf("q%d: %s\n", i, tokens2[i]);*/
		compPecas[i] = atoi(tokens1[i]);
		qtddPecas[i] = atoi(tokens2[i]);
	}

	Problema p = {n, m, maxComprimento, compPecas, qtddPecas};
	return p;

}
int main(int argc, char* argv[]) {
	//exemplo ./pcu prob03.txt 10 60 => 4 argumentos
	if (argc == 4) {
		//lógica no resto do programa
		printf("Executar durante %s segundos.\n", argv[3]);
		loadTest(argv[1], atoi(argv[2]));
	}



	return EXIT_SUCCESS;
}
