EXEC=jeu
CC=gcc

all: $(EXEC)

jeu: jeu.o ia.o
	$(CC) -o $@ $^

jeu.o: jeu.c jeu.h
	$(CC) -o $@ -c $<

ia.o: ia.c ia.h
	$(CC) -o $@ -c $<

clean:
	rm -rf *.o

mrproper: clean
	rm -rf jeu
