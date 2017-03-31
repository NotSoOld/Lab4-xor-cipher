all:

	gcc -o xor xor.c -fsanitize=address
	gcc -o gen gen.c -fsanitize=address
