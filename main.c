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
bool isValidMatrix(Problema p, int** matrix);
void generateSolution(Problema p, int** matrix, int* *ptSolution);
bool validSolution(Problema p, int** matrix, int* solution);

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

		int* solution;

		if (isValidMatrix(p, matrix)) {
			generateSolution(p, matrix, &solution);
			if (validSolution(p, matrix, solution)) {
				int waste = getWaste(p, matrix, solution);
				printf("Waste: %d\n", waste);
			}
		} else {
			printf("Matriz inválida\n");
		}


		destroyMatrix(p, &matrix);
		free(p.compPecas);
		free(p.qtddPecas);

	} else {
		printf("Utilização do programa: ./pcu [nome do teste] [n processos] [tempo em segundos]\n");
	}


	return EXIT_SUCCESS;
}

bool isValidMatrix(Problema p, int** matrix) {
	int line = 0;

	//percorrer por linha
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
			generatedValue = rand() % ((atual / compPeca) + 1);

			//garantir que a linha tem pelo menos um valor

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
	//verificação básica
	printf("[ ");
	int pecas = 0;
	for (int i = 0; i < p.n; i++) {
		printf("%d ", solution[i]);
		/* não tenho a certeza sobre isto
		if (solution[i] == 0) {
			printf(" Inválido\n");
			return false;
		}*/
		
		for (int j = 0; j < p.m; j++) {
			pecas += matrix[i][j] * solution[j];
		}

		if (pecas < p.qtddPecas[i]) {
			printf("Solução INVÁLIDA ]\n");
			return false;
		}
		pecas = 0;
	}
	printf("]\n");

	return true;
}

int getWaste(Problema p, int** matrix, int* solution) {
	int* wastePadrao = (int*) calloc(p.n, sizeof(int));
	int* wastePecas = (int*) calloc(p.m, sizeof(int));

	int waste = 0;
	int temp = 0;

	//Despredício nas quantidades de peças
	printf("Waste pecas:\n");
	for (int i = 0; i < p.m; i++) {
		for (int j = 0; j < p.n; j++) {
			wastePecas[i] += matrix[i][j] * solution[j];
		}
		wastePecas[i] = wastePecas[i] - p.qtddPecas[i];
		printf("%d\n", wastePecas[i]);
	}

	//Despredício nos padrões
	temp = 0;
	printf("Waste padrão:\n");
	for (int i = 0; i < p.n; i++) {
		for (int j = 0; j < p.m; j++) {
			//comprimento usado pelo padrão
			printf("Padrão %d, %d peça(s), comp: %d - %dm\n", i, matrix[j][i], p.compPecas[j], matrix[j][i] * p.compPecas[j]);
			temp += matrix[j][i] * p.compPecas[j];
		}
		printf("\n");
		
		wastePadrao[i] = p.maxComprimento - temp;
		temp = 0;
	}

	//Calcular despredício total
	printf("\nDespredício total:\n");
	for (int i = 0; i < p.m; i++) {
		printf("Peça %d, %dm, qttd:%d - %dm waste\n", i, p.compPecas[i], wastePecas[i], wastePecas[i] * p.compPecas[i]);
		printf("Padrão: %dm com %d peças\n\n", wastePadrao[i], p.qtddPecas[i]);

		waste += wastePecas[i] * p.compPecas[i];
		waste += wastePadrao[i] * p.qtddPecas[i];
	}


	free(wastePadrao);
	free(wastePecas);

	return waste;
}

int lineSum(Problema p, int** matrix, int* solution, int i) {
	int sum = 0;
	for (int j = 0; j < p.m; j++) {
		sum += matrix[i][j] * solution[j];
	}
	return sum;
}

void generateSolution(Problema p, int** matrix, int* *ptSolution) {
	//max
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
/*
	//min
	int* minValues = (int*) calloc(p.m, sizeof(int));
	int sum = 0;
	int min = 0;
	for (int i = 0; i < p.n; i++) {
		for (int j = 0; j < p.m; j++) {
			sum += matrix[i][j];
		}

		if (sum == 0) continue;

		//dividir qtddPecas por soma da linha e adicionar +1 caso a divisão não dá resto 0

		min = p.qtddPecas[i] / sum;

		if (p.qtddPecas[i] % sum != 0)
			min++;

		minValues[i] = min;

		sum = 0;
	}*/

	//gerar solução;
	int* solution = (int*) calloc(p.m, sizeof(int));

	//int range = 0;

	//solução gerada tem que ser válida
	int count = 0;
	do {
		
		for (int i = 0; i < p.m; i++) {
			printf("Zero - %d\n", zeros[i]);
			//caso haja só um valor na linha, a solução é facil de calcular
			//range = maxValues[i] - minValues[i];
			/*
			if (range == 0) {
				solution[i] = maxValues[i];
				continue;
			}*/

			//gera um valor entre [0, maxValue]
			//caso haja só um valor na linha vai "gerar" sempre o valor máximo
			solution[i] = rand() % (maxValues[i] + 1);

			//solution[i] = (rand() % (maxValues[i] - min)) + min + 1;
		}

		//facilmente verificar linhas só com um valor e corrigir-los para poupar tempo e evitar repetir a geração da solução.
		//int min = 0;
		for (int i = 0; i < p.n; i++) {
			/*
			if (zeros[i] == p.m - 1) {
				min = maxValues[i] - 1;
			} else {
				continue;
			}*/
			if (zeros[i] != p.m - 1) continue;

			//verificar se o valor para gerar é menor que o atual
			for (int j = 0; j < p.m; j++) {
				int n = matrix[i][j];
				if (n == 0) continue;

				int rand = p.qtddPecas[i] / n;
				if  (p.qtddPecas[i] % n != 0) {
					rand++;
				}
				//se for substituimos
				if (rand > solution[j]) {
					solution[j] = rand;
				}

			}


			//min = 0;
		}

		count++;

		if (count >= 1000) {
			printf("fuck\n");
			break;
		}
		
	} while (!validSolution(p, matrix, solution));

	free(maxValues);
	free(zeros);

	*ptSolution = solution;

	 //Quando encontra uma melhor solução (menos despredício)
	 //apartir da memória partilhada, (usando semáforos)
	 //substitui-se
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
