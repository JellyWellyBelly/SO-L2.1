#include "funcoesaux.h"

comando_t buffer_com[BUFFERSIZE_T];
int prodptr = 0;
int consptr = 0;

sem_t semaforo_p;
sem_t semaforo_c;

pthread_mutex_t trinco_p, trinco_c;
pthread_mutex_t vec_trinco[NUM_CONTAS];

int contador_simular = 0;

void init_vec_0(int *vec, int size) {
	int i = 0;

	while (i < size) {
		vec[i] = 0;
		i++;
	}
}

void push_pid(int *vec, int size, int pid) {
	int i = 0;

	while (i < size) {
		if (vec[i] == 0) {
			vec[i] = pid;
			return;
		}
		i++;
	}
}

int list_full(int *vec, int size) {
	int i = 0;

	while (i < size) {
		if (vec[i] == 0)
			return FALSE;
		i++;
	}
	return TRUE;
}

void init_trincos(int n_contas) {
	int i;

	for(i = 0; i < n_contas; i++)
		if(pthread_mutex_init(&vec_trinco[i], 0))
			exit(1);

	if(pthread_mutex_init(&trinco_p, 0))
        exit(1);
    
    if(pthread_mutex_init(&trinco_c, 0))
        exit(1);

    if(pthread_mutex_init(&trinco_simular, 0))
        exit(1);
}

void init_sem() {
    if(sem_init(&semaforo_p, 0, BUFFERSIZE_T))
        exit(1);
    if(sem_init(&semaforo_c, 0, 0))
        exit(1);
}

void init_log() {
	file = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0777);
	if (file < 0) {
		printf("NAO FOI POSSIVEL CRIAR/ABRIR O log.txt\n");
		exit(1);
	}
}

void cria_pool() {
	int i, err;

	for (i = 0; i < NUM_TRABALHADORAS; i++) {
		err = pthread_create(&tid[i], NULL, consome, NULL);
		if(err != 0) {
		    printf("ERRO NA CRIACAO DA POOL\n\n");
		 	exit(1);
		}
	}
}

void destroy() {
	int i = 0;

	while (i < NUM_CONTAS) {
		pthread_mutex_destroy(&vec_trinco[i]);
		i++;
	}

	if (pthread_mutex_destroy(&trinco_p) != 0) {
		printf("ERRO FATAL: mutex_destroy\n");
		exit(-1);
	}
    if (pthread_mutex_destroy(&trinco_c) != 0) {
    	printf("ERRO FATAL: mutex_destroy\n");
		exit(-1);
    }
    if (pthread_mutex_destroy(&trinco_simular) != 0) {
    	printf("ERRO FATAL: mutex_destroy\n");
		exit(-1);
    }

	if (sem_destroy(&semaforo_p) < 0) {
		printf("ERRO FATAL: sem_destroy\n");
		exit(-1);
	}
   	if (sem_destroy(&semaforo_c) <0) {
   		printf("ERRO FATAL: mutex_destroy\n");
		exit(-1);
	}
}

int obtem_valor_contador() {
	return contador_simular;
}

void atualiza_log(comando_t comando) {
	char text[MAXIMUM_STR_SIZE];
	int size;

	switch(comando.operacao) {

		case OP_CREDITAR: {
			size = snprintf(text, MAXIMUM_STR_SIZE, "%lu: %s %d %d\n", pthread_self(), COMANDO_CREDITAR, comando.idConta, comando.valor);
			break;
		}

		case OP_DEBITAR: {
			size = snprintf(text, MAXIMUM_STR_SIZE, "%lu: %s %d %d\n", pthread_self(), COMANDO_DEBITAR, comando.idConta, comando.valor);
			break;
		}

		case OP_LERSALDO: {
			size = snprintf(text, MAXIMUM_STR_SIZE, "%lu: %s %d\n", pthread_self(), COMANDO_LER_SALDO, comando.idConta);
			break;
		}

		case OP_TRANSFERIR: {
			size = snprintf(text, MAXIMUM_STR_SIZE, "%lu: %s %d %d %d\n", pthread_self(), COMANDO_TRANSFERIR, comando.idConta, comando.idContaDestino, comando.valor);
			break;
		}
	}

	if(write(file, text, size) < 0) {
		printf("NAO FOI POSSIVEL ATUALIZAR O log.txt\n\n");
		exit(1);
	}
}

void produz(comando_t comando) {

	if (sem_wait(&semaforo_p) < 0) {
		printf("ERRO FATAL: sem_wait\n");
		exit(-1);
	}
	if (pthread_mutex_lock(&trinco_p) < 0) {
		printf("ERRO FATAL: mutex_lock\n");
		exit(-1);
	}

	buffer_com[prodptr] = comando;
	prodptr = (prodptr + 1) % BUFFERSIZE_T;

	contador_simular++;

	if (pthread_mutex_unlock(&trinco_p) < 0) {
		printf("ERRO FATAL: mutex_unlock\n");
		exit(-1);
	}
	if (sem_post(&semaforo_c) < 0) {
		printf("ERRO FATAL: sem_post\n");
		exit(-1);
	}
} 

void *consome() {
	comando_t comando;

	sleep(5);

	while(TRUE) {
		if (sem_wait(&semaforo_c) < 0) {
			printf("ERRO FATAL: sem_wait\n");
			exit(-1);
		}
		if (pthread_mutex_lock(&trinco_c) < 0) {
			printf("ERRO FATAL: mutex_lock\n");
			exit(-1);
		}

		comando = buffer_com[consptr];
		consptr = (consptr + 1) % BUFFERSIZE_T;

		if (pthread_mutex_unlock(&trinco_c) < 0) {
			printf("ERRO FATAL: mutex_unlock\n");
			exit(-1);
		}
		if (sem_post(&semaforo_p) < 0) {
			printf("ERRO FATAL: sem_post\n");
			exit(-1);
		}

		trata_comando(comando);
		contador_simular--;
		if (pthread_cond_signal(&pode_simular) != 0) {
			printf("ERRO FATAL: pthread_cond_signal\n");
			exit(-1);
		}
	}

	return NULL;
}

void trata_comando(comando_t comando) {
	int idConta = comando.idConta,
		idContaDestino = comando.idContaDestino,
		valor = comando.valor;

	char text[MAXIMUM_STR_SIZE];
	char pipe_name[MAXIMUM_STR_SIZE];

	snprintf(pipe_name, MAXIMUM_STR_SIZE, "/tmp/i-banco-terminal-%d", comando.pid);
    int file_out = open(pipe_name, O_WRONLY, 0777);
    		
    if(file_out < 0) {
		printf("ERRO AO CRIAR/ABRIR O i-banco-terminal\n\n");
		exit(-1);
	}

	switch(comando.operacao) {

		case OP_DEBITAR: {
			
			if (pthread_mutex_lock(&vec_trinco[idConta]) < 0) {
				printf("ERRO FATAL: mutex_lock\n");
				exit(-1);
			}

            if (comando.valor < 0) {
                snprintf(text, MAXIMUM_STR_SIZE, "%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_DEBITAR);
                if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
            }

            if (!contaExiste(comando.idConta)) {
                snprintf(text, MAXIMUM_STR_SIZE, "%s(%d): Conta não existe.\n\n", COMANDO_DEBITAR, comando.idConta);
                if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
            }


			if (debitar(idConta, valor) < 0) {
	          	snprintf(text, MAXIMUM_STR_SIZE, "%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, idConta, valor);
	          	if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
			}
          	else {
              	snprintf(text, MAXIMUM_STR_SIZE, "%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
          		if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
          	}

			atualiza_log(comando);

			if (pthread_mutex_unlock(&vec_trinco[idConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_unlock\n");
				exit(-1);
			}

          	break;
          }

		case OP_CREDITAR: {

			if (pthread_mutex_lock(&vec_trinco[idConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_lock\n");
				exit(-1);
			}

			if (comando.valor < 0) {
                snprintf(text, MAXIMUM_STR_SIZE, "%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_CREDITAR);
                if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
            }

            if (!contaExiste(comando.idConta)) {
                snprintf(text, MAXIMUM_STR_SIZE, "%s(%d): Conta não existe.\n\n", COMANDO_CREDITAR, comando.idConta);
                if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
            }


			if (creditar(idConta, valor) < 0) {
                snprintf(text, MAXIMUM_STR_SIZE, "%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
            	if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
            }
            else {
                snprintf(text, MAXIMUM_STR_SIZE, "%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
            	if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
            }

			atualiza_log(comando);

            if (pthread_mutex_unlock(&vec_trinco[idConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_unlock\n");
				exit(-1);
			}

            break;
        }

        case OP_LERSALDO: {

        	if (pthread_mutex_lock(&vec_trinco[idConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_lock\n");
				exit(-1);
			}

			if (!contaExiste(comando.idConta)) {
            	snprintf(text, MAXIMUM_STR_SIZE, "%s(%d): Conta não existe.\n\n", COMANDO_LER_SALDO, comando.idConta);
            	if(write(file_out, text, sizeof(text)) < 0) {
            		printf("ERRO AO ENVIAR OUTPUT\n");
            		exit(-1);
           	 	}
       	 	}

        	int saldo = lerSaldo(idConta);

            snprintf(text, MAXIMUM_STR_SIZE, "%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
        	if(write(file_out, text, sizeof(text)) < 0) {
            	printf("ERRO AO ENVIAR OUTPUT\n");
            	exit(-1);
            }

			atualiza_log(comando);

            if (pthread_mutex_unlock(&vec_trinco[idConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_unlock\n");
				exit(-1);
			}

            break;
		}

		case OP_TRANSFERIR: {
			/* 
			    Quanto menor o id da conta, mais prioritaria é.
			    A prioridade da transferencia corresponde ao maximo
			   		das prioridades das contas envolventes 
			   	(i.e. conta com maior id).
			*/
			int menorConta;
			int maiorConta;

			if (idConta < idContaDestino) { /* Verificacao das prioridades */
				menorConta = idConta;
				maiorConta = idContaDestino;
			}
			else {
				menorConta = idContaDestino;
				maiorConta = idConta;
			}

			if (pthread_mutex_lock(&vec_trinco[menorConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_lock\n");
				exit(-1);
			}
			if (pthread_mutex_lock(&vec_trinco[maiorConta]) < 0) {
				printf("ERRO FATAL: pthread_mutex_lock\n");
				exit(-1);
			}

			if ((transferir(idConta, idContaDestino, valor) < 0) || (valor < 0) || (valor > lerSaldo(idConta))){
				snprintf(text, MAXIMUM_STR_SIZE, "Erro ao %s %d da conta %d para a conta %d.\n\n", COMANDO_TRANSFERIR, valor, idConta, idContaDestino);
				if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
			}
			else {
				snprintf(text, MAXIMUM_STR_SIZE, "%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, idConta, idContaDestino, valor);
				 if(write(file_out, text, sizeof(text)) < 0) {
                	printf("ERRO AO ENVIAR OUTPUT\n");
                	exit(-1);
                }
			}

			atualiza_log(comando);

			if (pthread_mutex_unlock(&vec_trinco[maiorConta]) < 0)  {
				printf("ERRO FATAL: pthread_mutex_unlock\n");
				exit(-1);
			}
			if (pthread_mutex_unlock(&vec_trinco[menorConta]) < 0)  {
				printf("ERRO FATAL: pthread_mutex_unlock\n");
				exit(-1);
			}
			break;
		}

        case OP_SAIR: {

        	pthread_exit(NULL);
        }
	}
	if (close(file_out) < 0) {
		printf("ERRO AO FECHAR UM FICHEIRO\n");
		exit(-1);
	}
}