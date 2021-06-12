void _start() {
	char str[] = "Hello c world!";
	__asm__ __volatile__ ("int $0x30" : : "a" (2), "b" (1), "c" (str), "d" (sizeof(str)));
}