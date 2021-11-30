#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
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
void getRandomMatrix(Problema p, int*** matrix);
void destroyMatrix(Problema p, int** *ptMatrix);

int main(int argc, char* argv[]) {
	srand(time(NULL));
	//exemplo ./pcu prob03.txt 10 60 => 4 argumentos
	if (argc == 4) {
		//lógica no resto do programa
		printf("Executar durante %s segundos.\n", argv[3]);
		Problema p = loadTest(argv[1], atoi(argv[2]));
		int** matrix;

		getRandomMatrix(p, &matrix);
		
		printf("\nMatriz gerada:\n");
		for (int i = 0; i < p.n; i++) {
			for (int j = 0; j < p.m; j++) {
				printf("%d ", matrix[i][j]);
			}
			printf("\n");
		}

		destroyMatrix(p, &matrix);
	} else {
		printf("Utilização do programa: ./pcu [nome do teste] [n processos] [tempo em segundos]\n");
	}


	return EXIT_SUCCESS;
}


void getRandomMatrix(Problema p, int** *ptMatrix){
	int** matrix = (int**) malloc(p.n * sizeof(int*));
	for (int i = 0; i < p.n; i++) {
		matrix[i] = (int*) malloc(p.m * sizeof(int));
	}

	int* compPecas = p.compPecas;
	
	for(int i = 0; i < p.n; i++){
		int atual = p.maxComprimento;
		int compPeca = 0;
		for(int j = 0; j < p.m; j++){
			//invalid read of size 4
			compPeca = compPecas[j];
			
			//Última posição do padrão tem que garantir
			//que utilize o resto do comprimento caso possível
			if (j == p.m - 1 && atual >= compPeca) {
				matrix[j][i] = atual / compPeca;
				continue;
			}


			//definimos o intervalo como quantas vezes uma peça
			//pode ir até o comprimento máximo
			int generatedValue = rand() % ((atual / compPeca) + 1);

			matrix[j][i] = generatedValue;
			atual = atual - (matrix[j][i] * compPeca);
			//printf("compPeca: %d, rand: %d, atual: %d\n", compPeca, generatedValue, atual);
		}
		//printf("\n");
	}

	*ptMatrix = matrix;
}

void destroyMatrix(Problema p, int** *ptMatrix) {
	int** matrix = *ptMatrix;

	if (matrix == NULL) return;

	for (int i = 0; i < p.n; i++) {
		free(matrix[i]);
	}
	free(matrix);
	*ptMatrix = NULL;
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
