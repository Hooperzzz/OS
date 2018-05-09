all: clean make

make:main.cpp my_print.o
	g++ -m32 -o main main.cpp my_print.o
my_print.o: my_print.asm
	nasm -f elf32 -o my_print.o my_print.asm
clean:
	rm -f my_print.o make