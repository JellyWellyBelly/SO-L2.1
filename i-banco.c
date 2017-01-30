/*
 * 
 *               I-BANCO
 * 
 *  Projeto SO - Exercicio 4, version 1
 *  Sistemas Operativos, DEI/IST/ULisboa 2016-17
 *  
 *  Co-autores: Tomas Carrasco nº 84774
 *              Miguel Viegas nº 84747
 *
 */

#include "mymacros.h"

int main () {

    int pid_list[MAXPROSS], status;
    char pipe_name[MAXIMUM_STR_SIZE];

    inicializarContas();
    init_vec_0(pid_list,MAXPROSS);

    cria_pool();
    init_sem();
    init_trincos(NUM_CONTAS);
    init_log();

    comando_t comando;
    int pid_terminal[NR_TERMINAL];
    init_vec_0(pid_terminal, NR_TERMINAL);

    printf("Bem-vinda/o ao i-banco\n\n");

	if(unlink(PIPE_SERVER) == -1) {
		if (errno != ENOENT) {
			printf("ERRO NA CRIACAO DO PIPE\n");
			exit(-1);
		}
	}

	if(mkfifo(PIPE_SERVER, 0777) == -1) {
			printf("ERRO NA CRIACAO DO PIPE\n");
			exit(-1);
	}

    int file_in = open(PIPE_SERVER, O_RDONLY, 0777);

    if(file_in < 0) {
    	printf("ERRO AO CRIAR/ABRIR i-banco-pipe\n");
    	exit(-1);
    }

    while (1) {

    	if (read(file_in, &comando, sizeof(comando)) < 0) {
    		printf("ERRO AO LER DO PIPE\n\n");
    		exit(-1);
    	}

    	if(comando.operacao == CRIA_LIGACAO) {
    		comando.operacao = ATRIBUTO_NULL;

    		push_pid(pid_terminal, NR_TERMINAL, comando.pid);

    		snprintf(pipe_name, MAXIMUM_STR_SIZE, "/tmp/i-banco-terminal-%d", comando.pid);
    		int file_out = open(pipe_name, O_WRONLY, 0777);
    		
    		if(file_out < 0) {
				printf("ERRO AO CRIAR/ABRIR O i-banco-terminal\n\n");
				exit(-1);
			}
    	}

        /*EOF (end of file) do stdin ou comando "sair"  TRATAR DESTA MERDA NO FIM*/
        else if (comando.operacao == OP_SAIR) {
            int i, pid_wait, tid_join;
            char ficheiro[MAXIMUM_STR_SIZE];

            for(i = 0; i < NUM_TRABALHADORAS; i++) /* Envio da operacao sair */
                produz(comando);

            comando.operacao = ATRIBUTO_NULL;

            for(i = 0; i < NUM_TRABALHADORAS; i++) { /* Terminio das tarefas trabalhadoras */
                tid_join = pthread_join(tid[i], NULL);
                if(tid_join == 0)
                    printf("TAREFA TERMINADA COM SUCESSO (TID=%ld)\n", tid[i]);
                else
                    printf("ERRO: TAREFA TERMINADA ABRUPTAMENTE\n");
            }

            for(i = 0; i < NR_TERMINAL; i++) {	/* Unlink de todos os pipes */
            	if (pid_terminal[i] != 0) {
            		snprintf(ficheiro, MAXIMUM_STR_SIZE, "/tmp/i-banco-terminal-%d", pid_terminal[i]);
            		if(unlink(ficheiro) == -1) {
						if (errno != ENOENT) {
							printf("ERRO DO SAIR: NAO CONSEGUI DESTRUIR UM PIPE\n");
							exit(-1);
						}
					}
				}	
            }

            if(unlink(PIPE_SERVER) == -1) { /* Destruicao do pipe principal */
				if (errno != ENOENT) {
					printf("ERRO DO SAIR: NAO CONSEGUI DESTRUIR O PIPE PRINCIPAL\n");
					exit(-1);
				}
			}
            
            destroy();

            printf("i-banco vai terminar.\n--\n");
            
            for(i = 0; i < MAXPROSS; i++)
                if(pid_list[i] != 0) {
                    if(comando.valor == SAIR_AGORA) /* excepcao do sair agora */
                        if (kill(pid_list[i], SIGUSR1) < 0) {
							printf("ERRO FATAL: kill\n");
							exit(-1);
						}
                    pid_wait = waitpid(pid_list[i], &status, 0);
                    if (pid_wait < 0) {
						printf("ERRO FATAL: waitpid\n");
						exit(-1);
					}
                    printf("FILHO TERMINADO (PID=%d; terminou %s)\n", pid_wait, (WIFEXITED(status)) ? "normalmente" : "abruptamente");
                }

            printf("--\ni-banco terminou.\n");

            if(close(file) < 0) {
             	printf("log.txt NAO FOI FECHADO CORRETAMENTE\n\n");
               	exit(1);
            }

            exit(EXIT_SUCCESS);                   
        }

        else if((comando.operacao == OP_DEBITAR) || /* Debitar */
        		(comando.operacao == OP_CREDITAR) ||/* Creditar */
        		(comando.operacao == OP_LERSALDO) ||/* Ler Saldo */
        		(comando.operacao == OP_TRANSFERIR))/* Transferir */
        {
            produz(comando);
            comando.operacao = ATRIBUTO_NULL;
        }

        /* Simular */
        else if (comando.operacao == OP_SIMULAR) {
            int pid;

			comando.operacao = ATRIBUTO_NULL;

			if (comando.valor < 0) {
                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_SIMULAR);
                continue;
            }

            if(list_full(pid_list, MAXPROSS) == TRUE) {
                printf("Erro: Demasiados processos.\n\n");
                continue;
            }

            if (pthread_mutex_lock(&trinco_simular) < 0) {
				printf("ERRO FATAL: pthread_mutex_lock\n");
				exit(-1);
			}
            while(obtem_valor_contador() > 0){
                if (pthread_cond_wait(&pode_simular, &trinco_simular) < 0) {
					printf("ERRO FATAL: pthread_cond_wait\n");
					exit(-1);
				}
            }
            if (pthread_mutex_unlock(&trinco_simular) < 0) {
				printf("ERRO FATAL: pthread_mutex_unlock\n");
				exit(-1);
			}

            pid = fork();

            if(pid < 0) {
                printf("Erro: Processo não criado.\n\n");
                exit(-1);
            }

            push_pid(pid_list, MAXPROSS, pid);

            if(pid == 0) {  /* Codigo do processo filho. */
                char file_name[MAXIMUM_STR_SIZE];

                snprintf(file_name, MAXIMUM_STR_SIZE, "i-banco-sim-%d.txt", getpid());
                file = open(file_name, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0666);
               
                if(file < 0) {
                	printf("NAO FOI POSSIVEL CRIAR/ABRIR O %s\n", file_name);
                	exit(1);
                }
                if (dup2(file, STDOUT_FILENO) < 0) {
					printf("ERRO FATAL: dup2\n");
					exit(-1);
				}
                simular(comando.valor);
            }
        }
    }
    return 0;
}