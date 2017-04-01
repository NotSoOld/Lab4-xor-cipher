regular:
	gcc -o gen gen.c
	gcc -o xor xor.c
	gcc -o unxor unxor.c

memsan:
	gcc -o gen_m gen.c -fsanitize=address
	gcc -o xor_m xor.c -fsanitize=address
	gcc -o unxor_m unxor.c -fsanitize=address
