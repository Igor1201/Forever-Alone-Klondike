/* 
   O pedaco de codigo a seguir foi mal escrito, considerando-se
   as regras do Igor e os padroes ANSI C.
 */

#include <stdio.h>
#include <time.h>
#include "constants.h"
#include "cores.h"
#include "logger.h"

FILE *arq = NULL;

int arqexists (void) {
	if (arq) return 1;
	return 0;
}

void msg (int f, char tipo, const char *m) {	
	char *t = NULL;
	switch (tipo) {
		case 'e':	// Erro
			cor1(VERMELHO);
			t = "ERRO:\t";
		break;
		case 'w':	// Warning
			cor1(AMARELO);
			t = "AVISO:\t";
		break;
		case 'i':	// Info
			cor1(AZUL);
			t = "INFO:\t";
		break;
		default:
			t = "";
		break;
	}

	printf("%s%s\n", t, m);
	if (f) fprintf(arq, "%s %s\n", t, m);
	decor();
}

void abrir_log (void) {
	if (!arq) arq = fopen(ARQUIVO_LOG, "a");
	if (!arq) { mensagem(0, 'w', "Nao foi possivel abrir o arquivo %s para escrita!", ARQUIVO_LOG); }
	else {
		time_t rawtime;
  		struct tm *timeinfo;
  		time(&rawtime);
  		timeinfo = localtime(&rawtime);
  		char buffer[80], buffer2[80];
  		strftime(buffer, 80, "%A, %d de %B de %Y", timeinfo);
  		strftime(buffer2, 80, "%H:%M:%S", timeinfo);
  		
  		mensagem(1, 'i', "----- ABRINDO LOG -----\n%s\nInicializado as %s\n(mais informacoes pertinentes aqui)\n", buffer, buffer2);
	}
}

void fechar_log (void) {
	logger('i', "----- FECHANDO LOG -----\n\n\n\n");
	fclose(arq);
}

