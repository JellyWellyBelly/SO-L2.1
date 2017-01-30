CFLAGS = -Wall -pedantic -g -c

all: i-banco i-banco-terminal

i-banco: funcoesaux.o contas.o i-banco.o
	gcc -pthread -lpthread -o i-banco funcoesaux.o contas.o i-banco.o

i-banco-terminal: commandlinereader.o contas.o i-banco-terminal.o
	gcc -pthread -lpthread -o i-banco-terminal commandlinereader.o contas.o i-banco-terminal.o

i-banco-terminal.o: i-banco-terminal.c mymacros.h
	gcc $(CFLAGS) i-banco-terminal.c

funcoesaux.o: funcoesaux.c funcoesaux.h mymacros.h
	gcc $(CFLAGS) funcoesaux.c

contas.o: contas.c contas.h mymacros.h
	gcc $(CFLAGS) contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h mymacros.h
	gcc $(CFLAGS) commandlinereader.c

i-banco.o: i-banco.c mymacros.h
	gcc $(CFLAGS) i-banco.c

zip: clean
	zip ProjetoSO-Final.zip *

clean:
	rm -f *.o *.txt *.zip i-banco i-banco-terminal
