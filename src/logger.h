#ifndef LOGGER_H
#define LOGGER_H

//#include <stdio.h>

#define convert(...) ({ char b[1000]; sprintf(b, __VA_ARGS__); b; })
#define mensagem(f, t, ...) msg(f, t, convert(__VA_ARGS__))
#define logger(...) mensagem(arqexists(), __VA_ARGS__)

int arqexists (void);
void abrir_log (void);
void fechar_log (void);
void msg (int f, char tipo, const char *m);

#endif

