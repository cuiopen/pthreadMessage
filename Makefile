main:main.o task/task.o fun/fun.o
	gcc -o main main.o task/task.o fun/fun.o -lpthread
main.o:main.c
	gcc -c main.c
task.o:task/task.c
	gcc -c task/task.c
fun.o:fun/fun.c
	gcc -c fun/fun.c
clean:
	rm -rf main *.o task/*.o fun/*.o

