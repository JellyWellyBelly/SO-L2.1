/*
// Operações sobre contas, versao 3
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#ifndef CONTAS_H
#define CONTAS_H

#include "mymacros.h"

void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int lerSaldo(int idConta);


/* * * * * * * * * * * * * * * * * * * * * * *\
 * Recebe: Tres inteiros.                    *
 * Retorna: Inteiro (booleano).              *
 * Descricao: Transfere dinheiro da conta de *
 *	origem para a conta destino.             *
\* * * * * * * * * * * * * * * * * * * * * * */
int transferir(int idContaOrigem, int idContaDestino, int valor);

/* * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Recebe: Um inteiro.                             *
 * Retorna: Nada.                                  *
 * Descricao: Simula a evolucao da conta ao fim de *
 *   'n' anos. A simulacao nao é alterada por      *
 *   outras funcoes do codigo-pai.                 *
\* * * * * * * * * * * * * * * * * * * * * * * * * */
void simular(int numAnos);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Recebe: Nada.                                             *
 * Retorna: Nada.                                            *
 * Descricao: Funcao que permite terminar o processo filho.  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void apanha_sinal();

#endif
