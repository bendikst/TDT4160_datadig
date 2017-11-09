#include <stdio.h>

void hello(char *who) {
	printf("Hei, %s!\n", who);
}

int main(void) {
	hello("folkens");
	return 0;
}
