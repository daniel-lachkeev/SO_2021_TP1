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

Problema loadTest(char* filename, int p);
Problema getProblem(char** lines);

int main(int argc, char* argv[]) {
	//exemplo ./pcu prob03.txt 10 60 => 4 argumentos
	if (argc == 4) {
		//lógica no resto do programa
		printf("Executar durante %s segundos.\n", argv[3]);
		loadTest(argv[1], atoi(argv[2]));
	} else {
		printf("Utilização do programa: ./pcu [nome do teste] [n processos] [tempo em segundos]\n");
	}

	return EXIT_SUCCESS;
}

Problema loadTest(char* filename, int p) {
	char filepath[100] = "";
	strcat(filepath, "cpu_tests/");
	strcat(filepath, filename);
	
	char** file = (char**)malloc(TEST_LINES * sizeof(char*));

	FILE* stream = fopen(filepath, "r");

	int count = 0;

	printf("%d processos a usar...\n", p);

	char line[1024];
	char* tmp;
	while (fgets(line, 1024, stream)) {
		if(count == TEST_LINES) break;

		tmp = strdup(line);
		int lineSize = sizeof(line)/sizeof(char);

		file[count] = (char*)malloc(lineSize);
		strcpy(file[count++], tmp);

		printf("%s", tmp);
		free(tmp);
	}
	printf("\n");
	Problema prob = getProblem(file);

	for (int i = 0; i < TEST_LINES; i++) {
		free(file[i]);
	}
	free(file);

	fclose(stream);

	return prob;
}

Problema getProblem(char** lines) {
	int n = atoi(lines[0]);
	int m = atoi(lines[1]);
	int maxComprimento = atoi(lines[2]);

	char** tokens1 = splitString(lines[3], m, " ");
	char** tokens2 = splitString(lines[4], m, " ");

	int compPecas[m];
	int qtddPecas[m];

	for(int i = 0; i < m; i++) {
		compPecas[i] = atoi(tokens1[i]);
		qtddPecas[i] = atoi(tokens2[i]);
	}

	free(tokens1);
	free(tokens2);

	Problema p = {n, m, maxComprimento, compPecas, qtddPecas};
	return p;
}
