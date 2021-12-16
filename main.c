#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "input.h"

#define TEST_LINES 5
#define TEST_PATH "pcu_tests/"

typedef struct solucao {
	int** matrizPadrao;

	int* maxValues;		//valores máximos para gerar
	int* zeros; 		//quantidade de zeros em cada linha
	int* vetorSolucao;

	int iterations;
	int score;
} Solucao;

typedef Solucao* PtSolucao;

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
int getWaste(Problema p, PtSolucao sol);
void destroyMatrix(Problema p, int** *ptMatrix);
bool isValidMatrix(Problema p, int** matrix);
PtSolucao generateSolution(Problema p);
bool isValidSolution(Problema p, int** matrix, int* solution);
void destroySolucao(Problema p, PtSolucao *ptSolucao);
void ajr_pe_algorithm(Problema p, PtSolucao sol);

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

		//solução inicial
		PtSolucao solution;

		solution = generateSolution(p);

		printf("\nMatriz inicial gerada:\n");
		for (int i = 0; i < p.n; i++) {
			for (int j = 0; j < p.m; j++) {
				printf("%d ", solution->matrizPadrao[i][j]);
			}
			printf("\n");
		}


		int waste = getWaste(p, solution);
		printf("Waste: %d\n", waste);
		

		destroySolucao(p, &solution);
		free(p.compPecas);
		free(p.qtddPecas);

	} else {
		printf("Utilização do programa: ./pcu [nome do teste] [n processos] [tempo em segundos]\n");
	}

	return EXIT_SUCCESS;
}

bool isValidMatrix(Problema p, int** matrix) {
	int line = 0;

	//percorrer por linha (getRandomMatrix já garante por coluna)
	for (int i = 0; i < p.n; i++) {
		for (int j = 0; j < p.m; j++) {
			line += matrix[i][j];
		}

		if (line == 0)
			return false;

		line = 0;
	}

	return true;
}


void getRandomMatrix(Problema p, int** *ptMatrix) {
	int** matrix = (int**) malloc(p.n * sizeof(int*));
	for (int i = 0; i < p.n; i++) {
		matrix[i] = (int*) malloc(p.m * sizeof(int));
	}

	int* compPecas = p.compPecas;
	
	for(int i = 0; i < p.n; i++){
		int atual = p.maxComprimento;
		int compPeca = 0;
		int generatedValue = 0;
		for(int j = 0; j < p.m; j++){
			compPeca = compPecas[j];
			
			//Última posição do padrão tem que garantir
			//que utilize o resto do comprimento caso possível
			if (j == p.m - 1 && atual >= compPeca) {
				matrix[j][i] = atual / compPeca;
				continue;
			}

			//definimos o intervalo como quantas vezes uma peça
			//pode ir até o comprimento máximo
			generatedValue = rand() % ((atual / compPeca) + 1);

			//garantir que a linha tem pelo menos um valor
			//TODO

			matrix[j][i] = generatedValue;
			atual = atual - (matrix[j][i] * compPeca);
		}
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

bool isValidSolution(Problema p, int** matrix, int* solution) {
	//verificação básica
	printf("[ ");
	int pecas = 0;
	for (int i = 0; i < p.n; i++) {
		//printf("%d ", solution[i]);

		for (int j = 0; j < p.m; j++) {
			pecas += matrix[i][j] * solution[j];
		}

		if (pecas < p.qtddPecas[i]) {
			//printf("Solução INVÁLIDA ]\n");
			return false;
		}
		pecas = 0;
	}
	//printf("]\n");

	return true;
}

int getWaste(Problema p, PtSolucao sol) {
	int* wastePadrao = (int*) calloc(p.n, sizeof(int));
	int* wastePecas = (int*) calloc(p.m, sizeof(int));

	int* solution = sol->vetorSolucao;
	int** matrix = sol->matrizPadrao;

	int waste = 0;
	int temp = 0;

	//Despredício nas quantidades de peças
	//printf("Waste pecas:\n");
	for (int i = 0; i < p.m; i++) {
		for (int j = 0; j < p.n; j++) {
			wastePecas[i] += matrix[i][j] * solution[j];
		}
		wastePecas[i] = wastePecas[i] - p.qtddPecas[i];
		//printf("%d\n", wastePecas[i]);
	}

	//Despredício nos padrões (se utiliza o comprimento máximo todo)
	temp = 0;
	//printf("Waste padrão:\n");
	for (int i = 0; i < p.n; i++) {
		for (int j = 0; j < p.m; j++) {
			//comprimento usado pelo padrão
			//printf("Padrão %d, %d peça(s), comp: %d - %dm\n", i, matrix[j][i], p.compPecas[j], matrix[j][i] * p.compPecas[j]);
			temp += matrix[j][i] * p.compPecas[j];
		}
		//printf("\n");
		
		wastePadrao[i] = p.maxComprimento - temp;
		temp = 0;
	}

	//Calcular despredício total
	printf("\nDespredício total:\n");
	for (int i = 0; i < p.m; i++) {
		//printf("Peça %d, %dm, qttd:%d - %dm waste\n", i, p.compPecas[i], wastePecas[i], wastePecas[i] * p.compPecas[i]);
		//printf("Padrão: %dm com %d peças\n\n", wastePadrao[i], p.qtddPecas[i]);

		waste += wastePecas[i] * p.compPecas[i];
		waste += wastePadrao[i] * p.qtddPecas[i];
	}

	free(wastePadrao);
	free(wastePecas);

	sol->score = waste;

	return waste;
}

int lineSum(Problema p, int** matrix, int* solution, int i) {
	int sum = 0;
	for (int j = 0; j < p.m; j++) {
		sum += matrix[i][j] * solution[j];
	}
	return sum;
}

void ajr_pe_algorithm(Problema p, PtSolucao sol) {
	int num = rand() % 5;

	if (num == 0) {
	//20% - gerar uma nova solução

	

	} else {
	//80% - altera um valor (que não seja o valor fixo)

	}

	 //Quando encontra uma melhor solução (menos despredício)
	 //apartir da memória partilhada, (usando semáforos)
	 //substitui-se
}

PtSolucao generateSolution(Problema p) {
	//gerar matriz padrão
	int** matrix;

	do {
		getRandomMatrix(p, &matrix);
	} while(!isValidMatrix(p, matrix));

	int* maxValues = (int*) calloc(p.m, sizeof(int));
	int* zeros = (int*) calloc(p.m, sizeof(int));

	for (int i = 0; i < p.n; i++) {
		int qtdd = p.qtddPecas[i];
		int value = 0;
		for (int j = 0; j < p.m; j++) {
			if (matrix[i][j] == 0) {
				zeros[i] += 1;
				continue;
			}

			value = qtdd / matrix[i][j];
			if (qtdd % matrix[i][j] != 0) 
				value++;

			if (value > maxValues[j]) {
				maxValues[j] = value;
			}
		}
	}

	//gerar solução;
	int* solution = (int*) calloc(p.m, sizeof(int));

	//solução gerada tem que ser válida
	int count = 0;
	do {
		
		//gerar valor
		for (int i = 0; i < p.m; i++) {
			solution[i] = rand() % (maxValues[i] + 1);
		}

		//verificar linhas só com um valor e corrigir-los para poupar tempo e evitar repetir a geração da solução.
		for (int i = 0; i < p.n; i++) {
			if (zeros[i] != p.m - 1) continue;

			//verificar se o valor para gerar é menor que o atual (inválido)
			for (int j = 0; j < p.m; j++) {
				int n = matrix[i][j];
				if (n == 0) continue;

				int rand = p.qtddPecas[i] / n;
				if  (p.qtddPecas[i] % n != 0) {
					rand++;
				}
				//se for maior substituimos
				if (rand > solution[j]) {
					solution[j] = rand;
				}
			}
		}

		//demasiadas iterações ao gerar, safety exit
		count++;
		if (count >= 1000) {
			printf("fuck\n");
			break;
		}
		
	} while (!isValidSolution(p, matrix, solution));

	PtSolucao sol = (PtSolucao) malloc(sizeof(Solucao));
	sol->matrizPadrao = matrix;
	sol->maxValues = maxValues;
	sol->vetorSolucao = solution;
	sol->zeros = zeros;
	sol->iterations = 0;
	sol->score = -1;

	return sol;
}

void destroySolucao(Problema p, PtSolucao *ptSolucao) {
	PtSolucao sol = *ptSolucao;
 
	if (sol == NULL) return;

	destroyMatrix(p, &sol->matrizPadrao);
	free(sol->maxValues);
	free(sol->vetorSolucao);
	free(sol->zeros);

	free(sol);
	*ptSolucao = NULL;
}


Problema loadTest(char* filename, int p) {
	//TODO simplificar o código

	char filepath[100] = "";
	strcat(filepath, TEST_PATH);
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

		//printf("%s", tmp);
		free(tmp);
	}
	//printf("\n");
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
	//printf("%d\n", p.qtddPecas[0]);
	return p;
}
