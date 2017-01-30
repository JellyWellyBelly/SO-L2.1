#include "contas.h"

int contasSaldos[NUM_CONTAS];
int flag = INATIVO;

int contaExiste(int idConta) {
	return (idConta > 0 && idConta <= NUM_CONTAS);
}

void inicializarContas() {
	int i;
	for (i=0; i<NUM_CONTAS; i++)
		contasSaldos[i] = 0;
}

int debitar(int idConta, int valor) {
	atrasar();
	if (contasSaldos[idConta - 1] < valor)
		return -1;

	contasSaldos[idConta - 1] -= valor;
	return 0;
}

int creditar(int idConta, int valor) {
	atrasar();
	contasSaldos[idConta - 1] += valor;
	return 0;
}

int lerSaldo(int idConta) {
	atrasar();
	return contasSaldos[idConta - 1];
}

int transferir(int idContaOrigem, int idContaDestino, int valor) {
	debitar(idContaOrigem, valor);
	creditar(idContaDestino, valor);
	return 0;
}

void apanha_sinal() {
	flag = ATIVO;
}

void simular(int numAnos) {
	int i, j, newValue[NUM_CONTAS];
	
	if (signal(SIGUSR1,apanha_sinal) == SIG_ERR) {
		printf("ERRO FATAL: signal\n");
		exit(-1);
	}

	for(j = 0; j < NUM_CONTAS; j++) {
		newValue[j] = lerSaldo(j+1);
	}

	for(i = 0; i <= numAnos; i++) {
		atrasar();
		
		if(flag == ATIVO) {
			printf("Simulacao terminada por sinal\n");
			exit(EXIT_SUCCESS);
		}

		printf("SIMULACAO: Ano %d\n", i);
		printf("=================\n");
		for(j = 0; j < NUM_CONTAS; j++) {
			if(newValue[j] >= 0)
				printf("Conta %d, Saldo %d\n", j+1, newValue[j]);
			else
				printf("Conta %d, Saldo 0\n", j+1);
			newValue[j] = (newValue[j] * (1 + TAXAJURO) - CUSTOMANUTENCAO);
		}
		printf("\n");
	}
	exit(EXIT_SUCCESS);
}
