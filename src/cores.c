#include <stdio.h>

void cor3 (int attr, int fg, int bg) {
	printf("%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
}

void cor2 (int fg, int bg) {
	printf("%c[1;%d;%dm", 0x1B, fg + 30, bg + 40);
}

void cor1 (int fg) {
	printf("%c[1;%dm", 0x1B, fg + 30);
}

void decor () {
	printf("%c[0m", 0x1B);
}

