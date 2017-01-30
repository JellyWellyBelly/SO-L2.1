#include "mymacros.h"

time_t start, stop;
char text[MAXIMUM_STR_SIZE];
char pipe_name[MAXIMUM_STR_SIZE];
char pipe_server[MAXIMUM_STR_SIZE];
int file_out, file_in;
comando_t comando;

void envia_comando();

int main(int argc, char** argv) {

	if (argc == 2) {
		strcpy(pipe_server, argv[1]);
	}
	else {
		printf("ERRO: SINTAXE INVALIDA (TERMINAL)\n");
		exit(-1);
	}

    char *args[MAXARGS + 1];
    char buffer[BUFFER_SIZE];


	comando.operacao = CRIA_LIGACAO;
	comando.pid = getpid();

	snprintf(pipe_name, MAXIMUM_STR_SIZE, "/tmp/i-banco-terminal-%d", getpid());

	printf("Bem-vinda/o ao i-banco-terminal\n\n");

	if(unlink(pipe_name) == -1) {
		if (errno != ENOENT) {
			printf("ERRO NA CRIACAO DO PIPE\n");
			exit(-1);
		}
	}

	file_in = open(pipe_server, O_WRONLY, 0777);
	if(file_out < 0) {
			printf("ERRO AO CRIAR/ABRIR O i-banco-terminal\n");
			exit(-1);
	}

    if(mkfifo(pipe_name, 0777) < 0) {
    	printf("ERRO NA CRIACAO DO PIPE\n");
    	exit(-1);
    }

    if(write(file_in, &comando, sizeof(comando)) < 0) { //cria a ligacao com o servidor
           	printf("ERRO NA LIGACAO AO SERVIDOR\n");
          	exit(-1);
    }

    file_out = open(pipe_name, O_RDONLY, 0777);
    if(file_out < 0) {
			printf("ERRO AO CRIAR/ABRIR O i-banco-terminal\n");
			exit(-1);
	}

    while (1) {

        int numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

		/* Nenhum argumento; ignora e volta a pedir */ 
        if (numargs == 0)
            continue;

        /* EOF (end of file) do stdin ou comando "sair" */
        else if (numargs < 0 || (numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {

            comando.operacao = OP_SAIR;

        	/* Comando Sair */
            if (numargs < 2) {
            	comando.valor = SAIR_NORMAL;
            }

            /* Comando Sair Agora */
            else {
                comando.valor = SAIR_AGORA;
            }

            if(write(file_in, &comando, sizeof(comando)) < 0) {
        		if (access(pipe_server, F_OK) == 0) {
    				printf("ERRO NO ENVIO DO COMANDO\n");
    				exit(-1);
    			}
    			else {
    				printf("Apenas sair-terminal sera aceite.\n");
    				continue;
    			}
            }

            if(close(file_in) < 0) {
            	printf("ERRO AO FECHAR O PIPE\n");
            	exit(-1);
            }
        }

        /* Debitar */
        else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
        	
        	if (numargs != 3) {
        		printf("%s: SINTAXE INVÁLIDA, TENTE DE NOVO.\n\n", COMANDO_DEBITAR);
        		continue;
        	}

            comando.operacao = OP_DEBITAR;
            comando.idConta = atoi(args[1]);
            comando.valor = atoi(args[2]);

            if (contaExiste(comando.idConta) != TRUE || comando.valor < 0) {
            	printf("%s: ARGUMENTOS INVALIDOS, TENTE DE NOVO.\n\n", COMANDO_DEBITAR);
            	continue;
            }

            envia_comando();
        }


        /* Creditar */
        else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {

        	if (numargs != 3) {
        		printf("%s: SINTAXE INVÁLIDA, TENTE DE NOVO.\n\n", COMANDO_CREDITAR);
        		continue;
        	}

        	comando.operacao = OP_CREDITAR;
            comando.idConta = atoi(args[1]);
            comando.valor = atoi(args[2]);

            if (contaExiste(comando.idConta) != TRUE || comando.valor < 0) {
            	printf("%s: ARGUMENTOS INVALIDOS, TENTE DE NOVO.\n\n", COMANDO_CREDITAR);
            	continue;
            }

            envia_comando();
        }


        /* Ler Saldo */
        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
        	
        	if (numargs != 2) {
        		printf("%s: SINTAXE INVÁLIDA, TENTE DE NOVO.\n\n", COMANDO_LER_SALDO);
        		continue;
        	}

            comando.operacao = OP_LERSALDO;
            comando.idConta = atoi(args[1]);

            if (contaExiste(comando.idConta) != TRUE) {
            	printf("%s: ARGUMENTOS INVALIDOS, TENTE DE NOVO.\n\n", COMANDO_LER_SALDO);
            	continue;
            }

            envia_comando();
        }


        /* Transferir */
        else if(strcmp(args[0], COMANDO_TRANSFERIR) == 0) {

        	if (numargs != 4) {
        		printf("%s: SINTAXE INVÁLIDA, TENTE DE NOVO.\n\n", COMANDO_TRANSFERIR);
        		continue;
        	}

            comando.operacao = OP_TRANSFERIR;
            comando.idConta = atoi(args[1]);
            comando.idContaDestino = atoi(args[2]);
            comando.valor = atoi(args[3]);

            if (contaExiste(comando.idConta) != TRUE || contaExiste(comando.idContaDestino) != TRUE || comando.valor < 0) {
            	printf("%s: ARGUMENTOS INVALIDOS, TENTE DE NOVO.\n\n", COMANDO_TRANSFERIR);
            	continue;
            }

            if(comando.idConta == comando.idContaDestino) {
                printf("%s: ARGUMENTOS INVALIDOS, TENTE DE NOVO.\n\n", COMANDO_TRANSFERIR);
            	continue;
            }

            envia_comando();
        }


        /* Simular */
        else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
        	
        	if (numargs != 2) {
        		printf("%s: SINTAXE INVÁLIDA, TENTE DE NOVO.\n\n", COMANDO_SIMULAR);
        		continue;
        	}

        	comando.operacao = OP_SIMULAR;
            comando.valor = atoi(args[1]);

            if (comando.valor < 0) {
            	printf("%s: ARGUMENTOS INVALIDOS, TENTE DE NOVO.\n\n", COMANDO_SIMULAR);
            	continue;
            }

            if(write(file_in, &comando, sizeof(comando)) < 0) {
            	if (access(pipe_server, F_OK) == 0) {
    				printf("ERRO NO ENVIO DO COMANDO\n");
    				exit(-1);
    			}
    			else {
    				printf("Apenas sair-terminal sera aceite.\n");
    				continue;
    			}
         	}
        }


        /* Comando Sair-terminal */
        else if (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0) {
        	
        	if (numargs != 1) {
        		printf("%s: SINTAXE INVÁLIDA, TENTE DE NOVO.\n\n", COMANDO_SAIR_TERMINAL);
        		continue;
        	}

        	if (close(file_in)) { //i-banco-pipe
        		if (errno != EBADF) {
        			printf("ERRO AO TERMINAR O %s\n", pipe_server);
            		exit(-1);
            	}
        	}
        	if (unlink(pipe_name)) { //i-banco-terminal-pid
        		if (errno != ENOENT) {
        			printf("ERRO AO TERMINAR O %s\n", pipe_name);
            		exit(-1);
            	}
        	}

        	comando.operacao = ATRIBUTO_NULL;
        	exit(EXIT_SUCCESS);
        }
        else {
            printf("COMANDO DESCONHECIDO. TENTE DE NOVO.\n\n");
        }
    }
    return 0;
}

void envia_comando() {

	time(&start);

	if(write(file_in, &comando, sizeof(comando)) < 0) {
    	if (access(pipe_server, F_OK) == 0) {
    		printf("ERRO NO ENVIO DO COMANDO\n");
    		exit(-1);
    	}
    	else {
    		printf("Apenas sair-terminal sera aceite.\n");
    		time(&stop);
    		return;
    	}
    }

    if(read(file_out, text, sizeof(text)) < 0) {
		printf("ERRO NA LEITURA DO i-banco-terminal");
		exit(-1);            	
    }

    time(&stop);

	printf("%s\n%f\n", text, difftime(stop, start));
}