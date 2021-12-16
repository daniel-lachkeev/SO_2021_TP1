#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include "input.h"

#define TEST_LINES 5
#define TEST_PATH "pcu_tests/"

//struct timeval tvi, tvf, tv_result;

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
PtSolucao copySolution(Problema p, PtSolucao sol);
PtSolucao generateSolution(Problema p, bool calculateScore);
bool isValidSolution(Problema p, int** matrix, int* solution);
void destroySolucao(Problema p, PtSolucao *ptSolucao);
void ajr_pe_algorithm(Problema p, PtSolucao *ptSol);
int* solutionChangeValue(Problema p, PtSolucao sol);
int* generateVectorSolution(Problema p, PtSolucao sol);


void printSolution(Problema p, PtSolucao sol) {
	for (int i = 0; i < p.n; i++) {
		for (int j = 0; j < p.m; j++) {
			printf("%d ", sol->matrizPadrao[i][j]);
		}
		printf("\n");
	}
	printf("\n[");

	for (int i = 0; i < p.m; i++) {
		printf("%d ", sol->vetorSolucao[i]);
	}
	printf("]\n");
}

int main(int argc, char* argv[]) {
	srand(time(NULL));

	//memória partilhada
	//iterações e tempo
	//solucao

	//exemplo ./pcu prob03.txt 10 60 => 4 argumentos
	if (argc == 4) {
		clock_t begin;
		double time_spent;
		double time_max = atof(argv[3]);

		//lógica no resto do programa
		printf("Executar durante %s segundos.\n", argv[3]);
		Problema p = loadTest(argv[1], atoi(argv[2]));

		//solução inicial
		PtSolucao solution;

		solution = generateSolution(p, true);
		

		//https://stackoverflow.com/questions/47252131/how-to-countdown-time-in-seconds-in-c
		//printf("i  n problem    m   ttime     eval   iterations   time   objects\n");
		begin = clock();
		while(1) {
			ajr_pe_algorithm(p, &solution);
        	time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
			if (time_spent >= time_max) {
				break;
			}
		}		

		printf("\nMatriz com melhor solução gerada:\n");
		printSolution(p, solution);
		printf("Waste: %d\n", solution->score);
		printf("Iterations: %d\n", solution->iterations);
		
		/*
		for (int i = 0; i < 20; i++) {
			int* newSol = solutionChangeValue(p, solution);
			for (int i = 0; i < p.m; i++) {
				printf("%d ", newSol[i]);
			}

			free(newSol);
			
			printf("\n");
			printSolution(p, solution);
		}*/
		

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
	int** matrix = (int**) calloc(p.n, sizeof(int*));
	for (int i = 0; i < p.n; i++) {
		matrix[i] = (int*) calloc(p.m, sizeof(int));
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
	//printf("[ ");
	int pecas = 0;
	int soma = 0;
	for (int i = 0; i < p.n; i++) {
		//printf("%d ", solution[i]);
		soma += solution[i];
		for (int j = 0; j < p.m; j++) {
			pecas += matrix[i][j] * solution[j];
		}

		if (pecas < p.qtddPecas[i]) {
			//printf("Solução INVÁLIDA ]\n");
			return false;
		}
		pecas = 0;
	}
	if (soma == 0) {
		return false;
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
	//printf("\nDespredício total:\n");
	for (int i = 0; i < p.m; i++) {
		//printf("Peça %d, %dm, qttd:%d - %dm waste\n", i, p.compPecas[i], wastePecas[i], wastePecas[i] * p.compPecas[i]);
		//printf("Padrão: %dm com %d peças\n\n", wastePadrao[i], p.qtddPecas[i]);

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

int* solutionChangeValue(Problema p, PtSolucao sol) {
    int* solution = (int*) calloc(p.m, sizeof(int));

    int** matrix = sol->matrizPadrao;
    int* zeros = sol->zeros;
	int* maxValues = sol->maxValues;

	//printf("zeros\n");
    //copiar os valores de sol->vectorSolucao
    for(int i = 0; i < p.m; i++) {
        solution[i] = sol->vetorSolucao[i];
		//printf("%d ", zeros[i]);
    }

    int pos = rand() % p.m;
	//printf("pos: %d\n", pos);
    int value = rand() % (maxValues[pos] +1);
	int diagonal = 0;
	//int iterations = 0;

	do {
		
		//linha a linha
		for(int j = 0; j < p.m; j++){
			if (diagonal == p.m) 
				return solution;

			if (zeros[j] == p.m - 1 && matrix[j][pos] != 0) {
				//verificar se é valor fixo
				int n = matrix[j][pos];
				int num = p.qtddPecas[pos] / n;
				if  (p.qtddPecas[pos] % n != 0) {
					num++;
				}

				//printf("%d\n", num);
				//se for igual, é valor fixo, passamos a frente no máximo p.m vezes
				if (num == maxValues[pos]) {
						pos = (pos + 1) % p.m;
						value = rand() % (maxValues[pos] +1);
						//printf("new pos: %d\n", pos);
						diagonal++;
				} else {
					break;
				}
			}
		}
		//printf("\n");
		
		solution[pos] = value;
/*
		iterations++;
		if (iterations >= 1000000) {
			//free(solution);
			//printf("pimba\n");
			break;
		}*/
    } while(!isValidSolution(p, matrix, solution));

    return solution;
}

void ajr_pe_algorithm(Problema p, PtSolucao *ptSol) {
	int num = rand() % 5;
	
	PtSolucao sol = *ptSol;
	sol->iterations++;
	PtSolucao newSol;
/*
	newSol = generateSolution(p, true);
	newSol->iterations = sol->iterations;
	
	if (newSol->score < sol->score) {
		*ptSol = newSol;
	}*/

	if (num == 0) {
	//20% - gerar uma nova solução e comparar
		newSol = generateSolution(p, true);
		newSol->iterations = sol->iterations;
		
		if (newSol->score < sol->score) {
			destroySolucao(p, ptSol);
			*ptSol = newSol;
		}

		
	} else {

	//80% - altera um valor (que não seja o valor fixo)
	/*
		int* newVectorSolution = solutionChangeValue(p, sol);

		//criar cópia da *ptSol e substituir a solução nova
		newSol = copySolution(p, sol);
		newSol->vetorSolucao = newVectorSolution;

		int newScore = getWaste(p, newSol);

		if (newScore < sol->score) {
			destroySolucao(p, ptSol);
			newSol->score = newScore;
			*ptSol = newSol;
		} else {
			destroySolucao(p, &newSol);
		}*/

		//optamos por criar um novo vetor solução inteiro em vez de mudar um só valor.
		int* newVectorSolution = generateVectorSolution(p, sol);
		newSol = copySolution(p, sol);
		newSol->vetorSolucao = newVectorSolution;

		int newScore = getWaste(p, newSol);

		if (newScore < sol->score) {
			destroySolucao(p, ptSol);
			newSol->score = newScore;
			*ptSol = newSol;
		} else {
			destroySolucao(p, &newSol);
		}

	}

	 //Quando encontra uma melhor solução (menos despredício)
	 //apartir da memória partilhada, (usando semáforos)
	 //substitui-se


}


PtSolucao copySolution(Problema p, PtSolucao sol) {
	//TODO
	PtSolucao newSol = (PtSolucao) malloc(sizeof(Solucao));

	//copiar matriz
	int** matrix = (int**) malloc(p.n * sizeof(int*));

	for (int i = 0; i < p.n; i++) {
		matrix[i] = (int*) malloc(p.m * sizeof(int));
		for (int j = 0; j < p.m; j++) {
			matrix[i][j] = sol->matrizPadrao[i][j];
		}
	}

	//copiar maxValues, zeros e vetorSolucao
	int* maxValues = (int*) calloc(p.m, sizeof(int));
	int* zeros = (int*) calloc(p.m, sizeof(int));
	int* vetorSolucao = (int*) calloc(p.m, sizeof(int));

	for (int i = 0; i < p.m; i++) {
		maxValues[i] = sol->maxValues[i];
		zeros[i] = sol->zeros[i];
		vetorSolucao[i] = sol->vetorSolucao[i];
	}

	newSol->matrizPadrao = matrix;
	newSol->maxValues = maxValues;
	newSol->vetorSolucao = vetorSolucao;
	newSol->zeros = zeros;
	newSol->iterations = sol->iterations;
	//newSol->score = getWaste(p, newSol);
	newSol->score = -1;

	return newSol;
}

int* generateVectorSolution(Problema p, PtSolucao sol) {

	int* solution = (int*) calloc(p.m, sizeof(int));

	int** matrix = sol->matrizPadrao;
	int* maxValues = sol->maxValues;
	int* zeros = sol->zeros;

	//solução gerada tem que ser válida
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
	} while (!isValidSolution(p, matrix, solution));

	return solution;

}

PtSolucao generateSolution(Problema p, bool calculateScore) {
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
	//int count = 0;
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
		/*
		count++;
		if (count >= 1000) {
			printf("fuck\n");
			break;
		}*/
		
	} while (!isValidSolution(p, matrix, solution));

	PtSolucao sol = (PtSolucao) malloc(sizeof(Solucao));
	sol->matrizPadrao = matrix;
	sol->maxValues = maxValues;
	sol->vetorSolucao = solution;
	sol->zeros = zeros;
	sol->iterations = 0;

	if (calculateScore)
		sol->score = getWaste(p, sol);
	else
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
	//*ptSolucao = NULL;
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
	//printf("n:%d\n", n);
	int m = atoi(lines[1]);
	//printf("m:%d\n", m);
	int maxComprimento = atoi(lines[2]);
	//printf("maxComprimento:%d\n", maxComprimento);

	char** tokens1 = splitString(lines[3], m, " ");
	char** tokens2 = splitString(lines[4], m, " ");

	//printf("pimba\n");
	int* compPecas = (int*)malloc(m * sizeof(int));
	int* qtddPecas = (int*)malloc(m * sizeof(int));

	//printf("pimba\n");
	for(int i = 0; i < m; i++) {
		compPecas[i] = atoi(tokens1[i]);
		qtddPecas[i] = atoi(tokens2[i]);
		//printf("compPeca: %d - qtddPeca: %d\n", compPecas[i], qtddPecas[i]);
	}

	free(tokens1);
	free(tokens2);

	Problema p = {n, m, maxComprimento, compPecas, qtddPecas};
	//printf("%d\n", p.qtddPecas[0]);
	return p;
}
