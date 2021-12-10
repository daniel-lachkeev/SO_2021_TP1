#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "input.h"

#define TEST_LINES 5
#define TEST_PATH "cpu_tests/"

typedef struct solucao {
	int** matrizPadrao;
	int* vetorSolucao;
} Solucao;

typedef struct problema {
	int n;
	int m;
	int maxComprimento;
	int* compPecas;
	int* qtddPecas;
} Problema;

Problema loadTest(char* filename, int p);
Problema getProblem(char** lines);
void getRandomMatrix(Problema p, int*** matrix);
int getWaste(Problema p, int** matrix, int* solution);
void destroyMatrix(Problema p, int** *ptMatrix);

int main(int argc, char* argv[]) {
	srand(time(NULL));

	//memória partilhada
	//iterações e tempo
	//solucao

	//exemplo ./pcu prob03.txt 10 60 => 4 argumentos
	if (argc == 4) {
		//lógica no resto do programa
		printf("Executar durante %s segundos.\n", argv[3]);
		Problema p = loadTest(argv[1], atoi(argv[2]));
		int** matrix;

		printf("\n%d", p.maxComprimento);
		printf("\n%d", p.qtddPecas[0]);

		getRandomMatrix(p, &matrix);
		
		printf("\nMatriz gerada:\n");
		for (int i = 0; i < p.n; i++) {
			for (int j = 0; j < p.m; j++) {
				printf("%d ", matrix[i][j]);
			}
			printf("\n");
		}

		int a, b, c = 0;
		scanf("%d", &a);
		scanf("%d", &b);
		scanf("%d", &c);

		int solution[3] = {a, b, c};

		int waste = getWaste(p, matrix, solution);
		printf("Waste: %d", waste);

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

bool validSolution(Problema p, int** matrix, int* solution) {
	return true;
}

int getWaste(Problema p, int** matrix, int* solution) {
	int waste = 0;
	int temp = 0;
	for (int i = 0; i < p.m; i++) {
		for (int j = 0; j < p.n; j++) {
			temp += (matrix[i][j] * solution[j]);
		}
		printf("%d\n", temp);
		waste += temp - p.qtddPecas[i];
		temp = 0;
		printf("%d\n", waste);
	}
	return waste;
}

void generateSolution(Problema p, int** matrix, int* *ptSolution) {
	
	/*  1 3 20
		3 0 2 | 20
		1 3 0 | 10
		0 0 1 | 20

		Determinar valor máximo
		arredondar sempre para cima
		max_linha_1 = [20 / 3 = 6,... = 7, 0, 20 / 10 10]
		max_linha_2 = [10, 4, 0]
		max_linha_3 = [0, 0, 20]
		max_global = [10, 4, 20] //escolhe-se sempre o valor mais alto da posição n de cada vetor
		geramos um vetor solução gerando valores aleatórios a partir de 0 até os valores máximos.


		[2, 3, 4] = [3x1 + 0x3 + 2x20 = 43, 1 x 1 + 3 x 3 + 0 x 20 = 10, 20]

	 */

/*
	int* solution = (int*) calloc(p.m, sizeof(int));

	for (int i = 0; i < p.m; i++) {
		int qtdd = p.qtddPecas[i];
		int max = INT_MAX;
		int min = 0;
		int sol = 0;
		
		for (int j = 0; j < p.n; j++) {
			if (matrix[i][j] == 0) continue;

			int n = 0;
			if (qtdd % matrix[i][j] != 0)
				n = 1;

			n = n + (qtdd / matrix[i][j]);
			if (n >= min) {
				min = n;
			}

			if (n <= max) {
				max = n;
			}

			if (max != min) {
				sol = (rand() % (max - min + 1)) + min;
			} else {
				sol = max;
			}
		}
		solution[i] = sol;
	}

	 //Quando encontra uma melhor solução (menos despredício)
	 //apartir da memória partilhada, (usando semáforos)
	 //substitui-se
	 *ptSolution = solution;*/
}

Problema loadTest(char* filename, int p) {
	char filepath[100] = "";
	strcat(filepath, "pcu_tests/");
	strcat(filepath, filename);
	
	char** file = (char**)malloc(TEST_LINES * sizeof(char*));

	FILE* stream = fopen(filepath, "r");
	if (stream == NULL)
		printf("Ficheiro/Caminho inválido");

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

	int* compPecas = (int*)malloc(m * sizeof(int));
	int* qtddPecas = (int*)malloc(m * sizeof(int));

	for(int i = 0; i < m; i++) {
		compPecas[i] = atoi(tokens1[i]);
		qtddPecas[i] = atoi(tokens2[i]);
	}

	free(tokens1);
	free(tokens2);

	Problema p = {n, m, maxComprimento, compPecas, qtddPecas};
	printf("%d\n", p.qtddPecas[0]);
	return p;
}
