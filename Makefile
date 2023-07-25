all:	mytar

main:	mytar.o
	gcc mytar.o -o mytar

main.o:
	gcc -c mytar.c -o mytar.o

clean:
	rm -f mytar mytar.o core *~
