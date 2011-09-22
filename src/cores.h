#ifndef CORES_H
#define CORES_H

#define RESET		0
#define BRIGHT 		1
#define DIM			2
#define UNDERLINE 	3
#define BLINK		4
#define REVERSE		7
#define HIDDEN		8

#define NORMAL		8
#define PRETO 		0
#define VERMELHO		1
#define VERDE		2
#define AMARELO		3
#define AZUL		4
#define MAGENTA		5
#define CIANO		6
#define BRANCO		7

void cor3 (int attr, int fg, int bg);
void cor2 (int fg, int bg);
void cor1 (int fg);
void decor (void);

#endif

