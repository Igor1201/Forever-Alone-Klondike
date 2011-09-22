// ------------------------- Default Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <math.h>

// ------------------------- Allegro Libraries
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

// ------------------------- SQLite Libraries
#include <sqlite3.h>

// ------------------------- My Libraries
#include "constants.h"
#include "cores.h"
#include "logger.h"

#define larg(x) al_get_bitmap_width(x)
#define alt(x) al_get_bitmap_height(x)
#define desenhar_t(i, t, x, y, w, h) al_draw_tinted_scaled_bitmap(i, t, 0.0, 0.0, larg(i), alt(i), x, y, w, h, 0)
#define desenhar(i, x, y, w, h) al_draw_scaled_bitmap(i, 0.0, 0.0, larg(i), alt(i), x, y, w, h, 0)
#define entre(ob, x, y) (ob >= x && ob <= y)
#define LARGURATELA al_get_display_width(tela)
#define ALTURATELA al_get_display_height(tela)
float V_WTELA = LARG_TELA;
float V_HTELA = ALT_TELA;
#define LARGURACARTA (V_WTELA/8.0)
#define ALTURACARTA ((LARGURACARTA*303.0)/209.0)
#define desback(x, y) if (!win || reset) { al_draw_filled_rounded_rectangle(x, y, x + LARGURACARTA, y + ALTURACARTA, 5.0, 5.0, al_map_rgba(0, 0, 0, 40)); }

typedef struct _carta {
	int numero;
	int naipe;
	ALLEGRO_BITMAP *imagem;
	int visivel;
	struct _carta *ant;
	struct _carta *prox;
} carta;

typedef struct _desfazer {
	int numerobar;
	int naipebar;
	carta **stack;
	int *qtd;
	struct _desfazer *ant;
} desfazer;

int iniciar_allegro (ALLEGRO_DISPLAY **, ALLEGRO_EVENT_QUEUE **, ALLEGRO_TIMER **);
void fechar_allegro (ALLEGRO_DISPLAY **, ALLEGRO_EVENT_QUEUE **, ALLEGRO_TIMER **);
void atualizar_tela (ALLEGRO_DISPLAY *, ALLEGRO_BITMAP **, float *, float *);
void criarbaralho (int, int);
carta *remover (int, carta **, int);
carta *removercont (int, int, int);
void adicionar (int, carta **);
int mover (int, int, carta **);
void distribuir (int);
void printar (int);
void copiar_estrutura (void);
void reestruturar (void);
void sqlinject (int, char *);
void sqlselect (void);
void sqltable (void);
int select_callback(void *, int, char **, char **);

carta *pilhasini[TOTAL], *pilhasfim[TOTAL], *blho;
int quantidade[TOTAL];
// ------------ CTRL + Z ------------
desfazer *controlz = NULL;
int reestruturou = 0;
// ----------------------------------
float pontos = 0;
ALLEGRO_FONT *font = NULL;
ALLEGRO_BITMAP *back = NULL;
// --------------------------- Highscore
sqlite3 *db;
char high5[5][100] = { "", "", "", "", "" };

int main (void) {
	abrir_log();
	ALLEGRO_DISPLAY *tela = NULL;
	ALLEGRO_EVENT_QUEUE *eventos = NULL;
	ALLEGRO_TIMER *timer = NULL;
	iniciar_allegro(&tela, &eventos, &timer);
	
	int ret = sqlite3_open("highscore.db", &db);
	if (ret != SQLITE_OK) logger('e', "SQL: Erro ao abrir o SQL!");
	else sqltable();
	
	ALLEGRO_BITMAP *fundo = al_create_bitmap(800, 600);
	font = al_load_ttf_font("resources/tempesta.ttf", 20, 0);
	
	int i, redraw = 0, dragonp = -1, win = 0, control = 0, aniq = 0, highscore = 0, showhigh = 0, reset = 0, help = 0;
	float mousex = 0.0, mousey = 0.0, mxnacarta = 0.0, mynacarta = 0.0, maish = 0.0, maisw = 0.0, aniy = 0.0, anix = 0.0, multip = 0.0, inicio = 0.0, savedtempo = 0.0;;
	carta *dragon = NULL, *dclick = NULL;
	double lastclick = 0.0;
	// ------------------ Highscore
	char nome[12] = "";
	int pos = 0;
	
	iniciar:
	i = 0; redraw = 0; dragonp = -1; win = 0; control = 0; aniq = 0; highscore = 0; showhigh = 0; reset = 0; help = 0;
	mousex = 0.0; mousey = 0.0; mxnacarta = 0.0; mynacarta = 0.0; maish = 0.0; maisw = 0.0; aniy = 0.0; anix = 0.0; multip = 0.0; inicio = al_get_time(); pontos = 0.0; savedtempo = 0.0;
	dragon = NULL, dclick = NULL;
	lastclick = 0.0;
	blho = NULL;
	pos = 0;
	
	for (i = 0; i < TOTAL; i++) { pilhasini[i] = NULL; pilhasfim[i] = NULL; quantidade[i] = 0; }
	
	criarbaralho(1, BARALHO);
	//printar(0);
	distribuir(BARALHO);
	
	// ------------ CTRL + Z ------------
	controlz = NULL;//(desfazer *) malloc(sizeof(desfazer));
	copiar_estrutura();
	//controlz->bar = NULL;
	//controlz->stack = pilhasini;
	//controlz->qtd = quantidade;
	//controlz->ant = NULL;
	// ----------------------------------
	
	atualizar_tela(tela, &fundo, &maish, &maisw);
	
	while (1) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(eventos, &ev);
		
		if (ev.type == ALLEGRO_EVENT_TIMER) {
			int r, ok = 1;
			for (r = OUT_MIN; r <= OUT_MAX; r++) {
				ok &= (pilhasfim[r] && pilhasfim[r]->numero == 13);				
			}
			
			if (win) {
				if (anix > V_WTELA) {
					aniq = (aniq == 3 ? 0 : aniq + 1);
					anix = 0.0;
					multip = (rand()%2?1:-1)*((rand()%2000)/1000.0 + 0.7);
				}
				
				float a = V_HTELA - (maish + (0.1 * ALTURACARTA));
				float plui = 55 * (a/80);
				aniy = a - (((10*a)/(anix+plui)) * abs((a/10)*cos((anix+plui)/(a/8))));
				anix += 4.8;
			}
			
			if (ok && !win) {
				//logger('i', "sasas");
				win = 1;
				anix = 0.0;
				multip = (rand()%2?1:-1)*((rand()%2000)/1000.0 + 0.7);
				savedtempo = al_get_time() - inicio;
			}
			redraw = 1;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			al_acknowledge_resize(ev.display.source);	
			atualizar_tela(tela, &fundo, &maish, &maisw);
			reset = 1;
			redraw = 1;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_EXPOSE) {
			al_acknowledge_resize(ev.display.source);	
			atualizar_tela(tela, &fundo, &maish, &maisw);
			//reset = 1;
			redraw = 1;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
			al_acknowledge_resize(ev.display.source);
			atualizar_tela(tela, &fundo, &maish, &maisw);
			//reset = 1;
			redraw = 1;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) break;
		else if (	ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
					ev.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY) {
			mousex = ev.mouse.x;
			mousey = ev.mouse.y;
		}
		else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			float x = 0.0, y = 0.0;
			
			if (!win) {
				if (blho) {
					x = maisw + (LARGURACARTA*1.25);
					y = maish + (0.1 * ALTURACARTA);
					
					if (entre(mousex, x, x + LARGURACARTA) && entre(mousey, y, y + ALTURACARTA) && blho->visivel) {
						dragon = blho;
						dragonp = BARALHO;
						mxnacarta = mousex - x; mynacarta = mousey - y;
						goto combobreaker1;
					}
				}
						
				int r;
				for (r = OUT_MIN; r <= OUT_MAX; r++) {
					if (pilhasfim[r]) {
						x = maisw + (LARGURACARTA*3.5) + (r - OUT_MIN) * (1.125 * LARGURACARTA);
						y = maish + (0.1 * ALTURACARTA);
						
						if (entre(mousex, x, x + LARGURACARTA) && entre(mousey, y, y + ALTURACARTA) && pilhasfim[r]->visivel) {
							dragon = pilhasfim[r];
							dragonp = r;
							mxnacarta = mousex - x; mynacarta = mousey - y;
							goto combobreaker1;
						}
					}
				}
				
				for (r = IN_MIN; r <= IN_MAX; r++) {
					float tam = (V_HTELA - (2.7 * ALTURACARTA)) / quantidade[r];
					tam = tam > 0.22 * ALTURACARTA ? 0.22 * ALTURACARTA : tam;
					
					carta *c = NULL;
					int cont = 1;
					for (c = pilhasfim[r]; c != NULL; c = c->ant) {
						x = maisw + (LARGURACARTA/8.0) + (r - IN_MIN) * (1.125 * LARGURACARTA); 
						y = maish + (1.6 * ALTURACARTA) + ((quantidade[r] - cont) * tam);
						
						if (entre(mousex, x, x + LARGURACARTA) && entre(mousey, y, y + ALTURACARTA) && c->visivel) {
							dragon = c;
							dragonp = r;
							mxnacarta = mousex - x; mynacarta = mousey - y;
							goto combobreaker1;
						}
					
						cont++;
					}	
				}
			}
			
			dragon = NULL;
			dragonp = -1;
			combobreaker1:
			redraw = 1;
		}
		else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			float x = 0.0, y = 0.0;
			
			if (!win) {
				// drag and drop
				if (dragon) {				
					int r;
					for (r = OUT_MIN; r <= OUT_MAX; r++) {
						x = maisw + (LARGURACARTA*3.5) + (r - OUT_MIN) * (1.125 * LARGURACARTA);
						y = maish + (0.1 * ALTURACARTA);
						
						if (entre(mousex, x, x + LARGURACARTA) && entre(mousey, y, y + ALTURACARTA) && (!dragon->prox || dragonp == BARALHO)) {
							carta *frog = NULL;
							if (dragonp == BARALHO) frog = blho->prox;
							
							copiar_estrutura();
							int k = mover(dragonp, r, &dragon);
							
							if (k) {
								if (dragonp == BARALHO) blho = frog;
								if (pilhasfim[dragonp]) {
									if (!pilhasfim[dragonp]->visivel) pontos += 5;
									pilhasfim[dragonp]->visivel = 1;
								}
								
								pontos += 10.0;
							}
							goto combobreaker2;
						}
					}
					
					for (r = IN_MIN; r <= IN_MAX; r++) {
						float tam = (V_HTELA - (2.7 * ALTURACARTA)) / quantidade[r];
						tam = tam > 0.22 * ALTURACARTA ? 0.22 * ALTURACARTA : tam;
						
						x = maisw + (LARGURACARTA/8.0) + (r - IN_MIN) * (1.125 * LARGURACARTA); 
						y = maish + (1.6 * ALTURACARTA) + ((quantidade[r] - 1) * tam);
							
						if (entre(mousex, x, x + LARGURACARTA) && entre(mousey, y, y + ALTURACARTA)) {
							if ((dclick) && (dclick == pilhasfim[r]) && (al_get_time() - lastclick <= 1.0)) {
								int w;
								for (w = OUT_MIN; w <= OUT_MAX; w++) {
									if ((!pilhasfim[w] && dclick->numero == 1) ||
										(pilhasfim[w] && dclick->numero - 1 == pilhasfim[w]->numero && dclick->naipe == pilhasfim[w]->naipe)) {
										copiar_estrutura();
										int k = mover(dragonp, w, &dclick);
										if (k) {
											if (pilhasfim[dragonp]) {
												if (!pilhasfim[dragonp]->visivel) pontos += 5;
												pilhasfim[dragonp]->visivel = 1;
											}
										}
										goto combobreaker2;
									}
								}
							}
							dclick = pilhasfim[r];
							lastclick = al_get_time();
							
							carta *frog = NULL;
							if (dragonp == BARALHO) frog = blho->prox;
							
							copiar_estrutura();
							int k = mover(dragonp, r, &dragon);
							
							if (k) {
								if (dragonp == BARALHO) blho = frog;
								if (pilhasfim[dragonp]) {
									if (!pilhasfim[dragonp]->visivel) pontos += 5;
									pilhasfim[dragonp]->visivel = 1;
								}
							}
							goto combobreaker2;
						}
					}
				}
				
				// not control+z, or YES, control+z
				if (entre(mousex, maisw + (LARGURACARTA*0.125), maisw + (LARGURACARTA*0.125) + LARGURACARTA) 
					&& entre(mousey, maish + (0.1 * ALTURACARTA),maish + (0.1 * ALTURACARTA) + ALTURACARTA)) {
					
					copiar_estrutura();
					
					if (blho) blho = blho->ant;
					else blho = pilhasfim[BARALHO];
					
					goto combobreaker2;
				}
				
				// double-click
				if (blho) {
					x = maisw + (LARGURACARTA*1.25);
					y = maish + (0.1 * ALTURACARTA);
					if (entre(mousex, x, x + LARGURACARTA) && entre(mousey, y, y + ALTURACARTA)) {
						if ((dclick) && (dclick == blho) && (al_get_time() - lastclick <= 1.0)) {
							int w;
							for (w = OUT_MIN; w <= OUT_MAX; w++) {
								if ((!pilhasfim[w] && dclick->numero == 1) ||
									(pilhasfim[w] && dclick->numero - 1 == pilhasfim[w]->numero && dclick->naipe == pilhasfim[w]->naipe)) {
										
									carta *frog = blho->prox;
							
									copiar_estrutura();
									int k = mover(dragonp, w, &dclick);
							
									if (k) {
										blho = frog;
										if (pilhasfim[dragonp]) {
											if (!pilhasfim[dragonp]->visivel) pontos += 5;
											pilhasfim[dragonp]->visivel = 1;
										}
									}
									
									goto combobreaker2;
								}
							}
						}
						dclick = blho;
						lastclick = al_get_time();
					}
				}
			}
			
			combobreaker2:
			redraw = 1;
			dragon = NULL;
			dragonp = -1;
		}
		else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch (ev.keyboard.keycode) {
				case ALLEGRO_KEY_LCTRL: case ALLEGRO_KEY_RCTRL:
					if (!win) control = 1;
				break;
				case ALLEGRO_KEY_BACKSPACE:
					if (highscore && pos > 0) nome[--pos] = '\x0';
				break;
			}
		}
		else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
			switch (ev.keyboard.keycode) {
				case ALLEGRO_KEY_LCTRL: case ALLEGRO_KEY_RCTRL:
					if (!win) control = 0;
				break;
				case ALLEGRO_KEY_ESCAPE:
					logger('i', "Saindo...");
					goto combobreaker;
				break;
				case ALLEGRO_KEY_Z:
					if (!win && control) {
						logger('i', "Reestruturando...");
						reestruturar();
					}
				break;
				case ALLEGRO_KEY_F2:
					logger('i', "Reiniciando...");
					goto iniciar;
				break;
				case ALLEGRO_KEY_F3:
					if (showhigh) showhigh = 0;
					else {
						sqlselect();
						showhigh = 1;
					}
				break;
				case ALLEGRO_KEY_H:
					help = !help;
				break;
				case ALLEGRO_KEY_ENTER:
					if (help) help = 0;
					else if (showhigh) showhigh = 0;
					else if (highscore) {
						sqlinject(pontos - 2*(((int)savedtempo)/10) + ((savedtempo>30)?(700000/savedtempo):0), nome);
						highscore = 0;
						win = 0;
						goto iniciar;
					}
					else if (win) highscore = 1;
				break;
				case ALLEGRO_KEY_SPACE:
					copiar_estrutura();
					if (blho) blho = blho->ant;
					else blho = pilhasfim[BARALHO];
					redraw = 1;
					dragon = NULL;
					dragonp = -1;
				break;
				//DEBUG MODE
				/*case ALLEGRO_KEY_F7:
					win = 1;
					anix = 0.0;
					multip = (rand()%2?1:-1)*((rand()%2000)/1000.0 + 0.7);
					savedtempo = al_get_time() - inicio;
				break;*/
			}
			continue;
			combobreaker: break;
		}
		else if (ev.type == ALLEGRO_EVENT_KEY_CHAR && highscore && pos <= 11
		&& ev.keyboard.unichar != '\x8' && ev.keyboard.unichar != 13) {
			nome[pos++] = ev.keyboard.unichar;
		}
 
		if (redraw && al_is_event_queue_empty(eventos)) {
			
			al_set_target_bitmap(fundo);
			
			if (!win || reset) al_clear_to_color(al_map_rgb(0, 128, 0));
			
			// -------------------- BACKS
			desback(maisw + (LARGURACARTA*1.25), maish + (0.1 * ALTURACARTA));
			desback(maisw + (LARGURACARTA*0.125), maish + (0.1 * ALTURACARTA));
			
			int r, glow = 0;
			if (blho) {
				if (dragon == blho) glow = 1;
				desenhar_t(blho->imagem, al_map_rgba_f(!glow, !glow, !glow, !glow), maisw + (LARGURACARTA*1.25), 
					maish + (0.1 * ALTURACARTA), LARGURACARTA, ALTURACARTA);
			}
			
			if (blho != pilhasini[BARALHO])
					desenhar(back, maisw + (LARGURACARTA*0.125), 
						maish + (0.1 * ALTURACARTA), LARGURACARTA, ALTURACARTA);
			if (win && pilhasfim[OUT_MIN + aniq])
				desenhar_t(pilhasfim[OUT_MIN + aniq]->imagem, al_map_rgba_f(!glow, !glow, !glow, !glow), ((multip)*anix) + maisw + (LARGURACARTA*3.5) + (aniq) * (1.125 * LARGURACARTA), 
						aniy + maish + (0.1 * ALTURACARTA), LARGURACARTA, ALTURACARTA);
			else for (r = OUT_MIN; r <= OUT_MAX; r++) {
				desback(maisw + (LARGURACARTA*3.5) + (r - OUT_MIN) * (1.125 * LARGURACARTA), maish + (0.1 * ALTURACARTA));
				
				if (dragon == pilhasfim[r]) glow = 1;
				else glow = 0;
				if (pilhasfim[r])
					desenhar_t(pilhasfim[r]->imagem, al_map_rgba_f(!glow, !glow, !glow, !glow), maisw + (LARGURACARTA*3.5) + (r - OUT_MIN) * (1.125 * LARGURACARTA), 
						maish + (0.1 * ALTURACARTA), LARGURACARTA, ALTURACARTA);
			}
			for (r = IN_MIN; r <= IN_MAX; r++) {
				float tam = (V_HTELA - (2.7 * ALTURACARTA)) / quantidade[r];
				tam = tam > 0.22 * ALTURACARTA ? 0.22 * ALTURACARTA : tam;
				carta *c = NULL;
				int cont = 0;
				glow = 0;
				
				desback(maisw + (LARGURACARTA/8.0) + (r - IN_MIN) * (1.125 * LARGURACARTA), maish + (1.6 * ALTURACARTA) + (cont * tam));
				
				for (c = pilhasini[r]; c != NULL; c = c->prox) {
					if (dragon == c) glow = 1;
					desenhar_t((c->visivel?c->imagem:back), al_map_rgba_f(!glow, !glow, !glow, !glow), maisw + (LARGURACARTA/8.0) + (r - IN_MIN) * (1.125 * LARGURACARTA), 
						maish + (1.6 * ALTURACARTA) + (cont * tam), LARGURACARTA, ALTURACARTA);
					cont++;
				}
			}
			if (dragon) {
				carta *c = NULL;
				int cont = 0;
				for (c = dragon; c; c = c->prox) {
					desenhar(c->imagem, mousex - mxnacarta, mousey + (cont * 0.22 * ALTURACARTA) - mynacarta, LARGURACARTA, ALTURACARTA);
					cont++;
					if (dragonp == BARALHO) break;
				}
			}
			
			al_draw_filled_rectangle(0, ALTURATELA - 30, LARGURATELA, ALTURATELA, al_map_rgba(127, 127, 127, 200));
			al_draw_textf(font, al_map_rgb(0, 0, 0), 10.0, ALTURATELA - 30.0, ALLEGRO_ALIGN_LEFT, "Tempo: %.0f / Pontos: %.0f", al_get_time() - inicio, pontos - 2*(((int)(al_get_time() - inicio))/10));
			al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA - 10.0, ALTURATELA - 30.0, ALLEGRO_ALIGN_RIGHT, "H: ajuda");
			
			if (highscore) {
				
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 40, LARGURATELA/2 + 230, ALTURATELA/2 + 40, al_map_rgba(127, 127, 127, 200));
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 40, LARGURATELA/2 + 230, ALTURATELA/2 - 10, al_map_rgba(127, 127, 127, 100));
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 40, ALLEGRO_ALIGN_CENTRE, "Highscore");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 10, ALLEGRO_ALIGN_CENTRE, "Digite seu nome:");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 + 10, ALLEGRO_ALIGN_CENTRE, "%s", nome);
				al_draw_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 40, LARGURATELA/2 + 230, ALTURATELA/2 + 40, al_map_rgba(0, 0, 0, 255), 3.0);
			}
			else if (win) {
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 + 70, al_map_rgba(127, 127, 127, 200));
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 - 40, al_map_rgba(127, 127, 127, 100));
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 70, ALLEGRO_ALIGN_CENTRE, "YOU WIN");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 40, ALLEGRO_ALIGN_CENTRE, "%.2f segundos", savedtempo);
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 20, ALLEGRO_ALIGN_CENTRE, "Final Score: %.0f pontos + %.0f bonus!", pontos - 2*(((int)savedtempo)/10), (((savedtempo>30)?(700000/savedtempo):0)));
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 + 40, ALLEGRO_ALIGN_CENTRE, "Aperte ENTER para salvar highscore");
				al_draw_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 + 70, al_map_rgba(0, 0, 0, 255), 3.0);
			}
			else if (showhigh) {
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 + 70, al_map_rgba(127, 127, 127, 200));
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 - 40, al_map_rgba(127, 127, 127, 100));
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 70, ALLEGRO_ALIGN_CENTRE, "HIGHSCORES");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 40, ALLEGRO_ALIGN_CENTRE, "%s", high5[0]);
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 20, ALLEGRO_ALIGN_CENTRE, "%s", high5[1]);
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2, ALLEGRO_ALIGN_CENTRE, "%s", high5[2]);
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 + 20, ALLEGRO_ALIGN_CENTRE, "%s", high5[3]);
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 + 40, ALLEGRO_ALIGN_CENTRE, "%s", high5[4]);
				al_draw_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 + 70, al_map_rgba(0, 0, 0, 255), 3.0);
			}
			else if (help) {
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 + 70, al_map_rgba(127, 127, 127, 200));
				al_draw_filled_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 - 40, al_map_rgba(127, 127, 127, 100));
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 70, ALLEGRO_ALIGN_CENTRE, "AJUDA");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 40, ALLEGRO_ALIGN_CENTRE, "H : Esta janela");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 - 20, ALLEGRO_ALIGN_CENTRE, "F2 : Novo Jogo");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2, ALLEGRO_ALIGN_CENTRE, "F3 : Highscores");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 + 20, ALLEGRO_ALIGN_CENTRE, "SPACE : Virar monte");
				al_draw_textf(font, al_map_rgb(0, 0, 0), LARGURATELA/2, ALTURATELA/2 + 40, ALLEGRO_ALIGN_CENTRE, "CONTROL + Z : Desfazer");
				al_draw_rectangle(LARGURATELA/2 - 230, ALTURATELA/2 - 70, LARGURATELA/2 + 230, ALTURATELA/2 + 70, al_map_rgba(0, 0, 0, 255), 3.0);
			}
			
			
			al_set_target_bitmap(al_get_backbuffer(tela));
			al_draw_scaled_bitmap(fundo, 0.0, 0.0, LARGURATELA, ALTURATELA, 0.0, 0.0, LARGURATELA, ALTURATELA, 0);
			al_flip_display();
			
			redraw = 0;
			reset = 0;
		}
	}
	
	fechar_allegro(&tela, &eventos, &timer);
	sqlite3_close(db);	
	fechar_log();
	return 0;
}

carta *removercont (int r, int cont, int single) {
	carta *tmp = pilhasfim[r];
	while (cont > 0 && tmp != NULL) {
		tmp = tmp->ant;
		cont--;
	}
	return remover(r, &tmp, single);
}

carta *remover (int r, carta **c, int single) {
	if (*c) {
		if (single) {
			if ((*c)->ant) (*c)->ant->prox = (*c)->prox;
			else pilhasini[r] = (*c)->prox;
			
			if ((*c)->prox) (*c)->prox->ant = (*c)->ant;
			else pilhasfim[r] = (*c)->ant;
			
			(*c)->prox = NULL;
			quantidade[r]--;
		}
		else {
			if ((*c)->ant) (*c)->ant->prox = NULL;
			else pilhasini[r] = NULL;
			pilhasfim[r] = (*c)->ant;
			
			carta *d = *c;
			while (d) { quantidade[r]--; d = d->prox; }
		}
		
		(*c)->ant = NULL;
	}
	else logger('w', "Carta nao encontrada!");
	
	return *c;
}

void adicionar (int r, carta **c) {
	if (pilhasfim[r]) (pilhasfim[r])->prox = *c;
	else pilhasini[r] = *c;
	
	(*c)->ant = pilhasfim[r];
	pilhasfim[r] = *c;
	
	quantidade[r]++;
	while ((pilhasfim[r])->prox != NULL) {
		pilhasfim[r] = (pilhasfim[r])->prox;
		quantidade[r]++;
	}
}

void printar (int r) {
	carta *b = NULL;
	int i = 1;
	for (b = pilhasini[r]; b != NULL; b = b->prox) {
		printf("%d [", i);
		cor1(VERMELHO);
		if (b->ant) printf("%d_%d", b->ant->numero, b->ant->naipe);
		else printf("NULL");
		decor();
		printf("][%d_%d][", b->numero, b->naipe);
		cor1(AZUL);
		if (b->prox) printf("%d_%d", b->prox->numero, b->prox->naipe);
		else printf("NULL");
		decor();
		printf("]\n");
		i++;
	}
}

void criarbaralho (int tmp, int r) {
	logger('o', "Criando baralho...");
	int i, j;
	char path[100];
	
	for (i = 0; i < 4; i++) {
		for (j = 1; j <= 13; j++) {
			carta *c = (carta *) malloc(sizeof(carta));
			c->numero = j;
			c->naipe = i;
			sprintf(path, "resources/cartas/%d_%d.png", j, i);
			c->imagem = al_load_bitmap(path);
			c->visivel = 0;
			c->ant = NULL;
			c->prox = NULL;
			adicionar(tmp, &c);
		}
	}
	
	back = al_load_bitmap("resources/cartas/back.png");
	
	//printar(b);
	
	logger('o', "Embaralhando...");
	srand(time(NULL));
	
	for (i = 52; i >= 1; i--) {
		carta *d = removercont(tmp, rand()%i, 1);
		adicionar(r, &d);
	}
}

int mover (int d, int p, carta **c) {
	int out = entre(p, OUT_MIN, OUT_MAX);
	int eb = (d == BARALHO);
	carta *de = pilhasfim[d];
	carta *para = pilhasfim[p];
	if (de) {
		int a = !para; // se a = 0, para existe
		
		if (out) {
			if ((a && (*c)->numero == 1) || (!a && ((*c)->numero - 1 == para->numero) && ((*c)->naipe == para->naipe))) {
				carta *removed = eb ? remover(d, &(*c), 1) : remover(d, &(*c), 0);
				adicionar(p, &removed);
				pontos += 10;
				return 1;
			}
		}
		else {
			if ((a && (*c)->numero == 13) || (!a && (*c)->numero + 1 == para->numero && (*c)->naipe%2 != para->naipe%2)) {
				carta *removed = eb ? remover(d, &(*c), 1) : remover(d, &(*c), 0);
				adicionar(p, &removed);
				if (!d) pontos += 5;
				else if (entre(d, OUT_MIN, OUT_MAX)) pontos -= 15;
				return 1;
			}
		}
	}
	
	// free controlz
	if (controlz) controlz = controlz->ant;
	return 0;
}

void distribuir (int r) {
	int i;
	for (i = IN_MIN; i <= IN_MAX; i++) {
		carta *removed = removercont(r, (i - IN_MIN), 0);
		adicionar(i, &removed);
		pilhasfim[i]->visivel = 1;
	}
	
	carta *e;
	for (e = pilhasini[r]; e != NULL; e = e->prox) {
		e->visivel = 1;
	}
}

void copiar_estrutura (void) {
	logger('i', "Copiando estrutura...");
	
	if (reestruturou) {
		reestruturou = 0;
		// 'deleta' os controlz antigos.
		//controlz = NULL;
	}
	
	desfazer *novo = (desfazer *) malloc(sizeof(desfazer));
	novo->ant = controlz;
	
	carta **pini = (carta **) malloc (sizeof(carta) * 12);
	carta **pfim = (carta **) malloc (sizeof(carta) * 12);
	int *quan = (int *) malloc (sizeof(int) * 12);
	carta *j = NULL;
	int i, first = 1;
	
	//if (!blho) novo->bar = NULL;
	novo->numerobar = 0;
	
	for (i = 0; i < TOTAL; i++) {
		pini[i] = NULL;
		pfim[i] = NULL;
		quan[i] = quantidade[i];
		
		for (j = pilhasini[i]; j; j = j->prox) {
			carta *d = (carta *) malloc(sizeof(carta));
			d->numero = j->numero;
			d->naipe = j->naipe;
			d->imagem = j->imagem;
			d->visivel = j->visivel;
			d->ant = pfim[i];
			d->prox = NULL;
			
			if (blho == j) {
				novo->numerobar = j->numero;
				novo->naipebar = j->naipe;
			}
			
			if (pfim[i]) pfim[i]->prox = d;
			pfim[i] = d;
			
			if (first) {
				pini[i] = d;
				first = 0;
			}
		}
		first = 1;
	}
	
	novo->stack = pini;
	novo->qtd = quan;
	controlz = novo;
}

void reestruturar (void) {
	int i;
	if (controlz) {
		if (controlz->numerobar == 0) blho = NULL;
		
		for (i = 0; i < TOTAL; i++) {
			pilhasini[i] = controlz->stack[i];
			
			carta *j = controlz->stack[i];
			carta *ult = NULL;
			if (j) {
				while (j) {
					if (i == BARALHO && j->numero == controlz->numerobar && j->naipe == controlz->naipebar) blho = j;
					ult = j;
					j = j->prox;
				}
			}
			pilhasfim[i] = ult;
			
			quantidade[i] = controlz->qtd[i];
		}
		
		pontos -= 15;
		
		//desfazer *def = controlz;
		controlz = controlz->ant;
		reestruturou = 1;
		
		// -------------- FREEEEEEEEEEE WILLYYYYY!!!
		
		/*int k;
		for (k = 0; k < TOTAL; k++) {
			carta *u = NULL, *tmp = NULL;
			for (u = def->stack[k]; u; u = u->prox) {
				if (tmp) free(tmp);
				tmp = u;
			}
		}
		free(def->qtd);
		free(def);*/
	}
}

void atualizar_tela (ALLEGRO_DISPLAY *tela, ALLEGRO_BITMAP **fundo, float *maish, float *maisw) {
	float h = (float)al_get_display_height(tela), w = (float)al_get_display_width(tela);
	
	if (h/w > 0.75) {
		V_WTELA = w;
		V_HTELA = (3.0 * V_WTELA) / 4.0 - (30.0 + (0.1 * ALTURACARTA));
	}
	else if (h/w < 0.75) {
		V_HTELA = h - (30.0 + (0.1 * ALTURACARTA));
		V_WTELA = (4.0 * V_HTELA) / 3.0;
	}
	
	*fundo = al_create_bitmap(LARGURATELA, ALTURATELA);
	*maish = (ALTURATELA - V_HTELA)/2.0;
	*maisw = (LARGURATELA - V_WTELA)/2.0;
}

int iniciar_allegro (ALLEGRO_DISPLAY **tela, ALLEGRO_EVENT_QUEUE **eventos, ALLEGRO_TIMER **timer) {
	if (!al_init()) {
		logger('e', "Falha ao iniciar o Allegro!");
		return -1;
	}
	
	/*if (!al_install_audio()) {
		logger('e', "Falha ao iniciar o som!");
		return -1;
	}
 
	if (!al_init_acodec_addon()) {
		logger('e', "Falha ao iniciar os codecs de audio!");
		return -1;
	}
 
	if (!al_reserve_samples(NUM_SONS)) {
		logger('e', "Falha ao reservar as amostras!");
		return -1;
	}
	*/
	if (!al_init_image_addon()) {
		logger('e', "Falha ao iniciar o addon de imagens!");
		return -1;
	}
	
	al_init_primitives_addon();
	
	al_init_font_addon();
	
	if (!al_init_ttf_addon()) {
		logger('e', "Falha ao iniciar as fontes TTF!");
		return -1;
	}
 
	if (!al_install_keyboard()) {
		logger('e', "Falha ao iniciar o teclado!");
		return -1;
	}
 
	if (!al_install_mouse()) {
		logger('e', "Falha ao iniciar o mouse!");
		return -1;
	}
 
	*timer = al_create_timer(1.0 / FPS);
	if (!(*timer)) {
		logger('e', "Falha ao criar o timer!");
		return -1;
	}
	
	al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE | ALLEGRO_GENERATE_EXPOSE_EVENTS);	
	*tela = al_create_display(LARG_TELA, ALT_TELA);
	if (!(*tela)) {
		logger('e', "Falha ao criar a tela!");
		al_destroy_timer(*timer);
		return -1;
	}
	
	*eventos = al_create_event_queue();
	if (!(*eventos)) {
		logger('e', "Falha ao criar os eventos!");
		//al_destroy_bitmap(bouncer);
		al_destroy_display(*tela);
		al_destroy_timer(*timer);
		return -1;
	}
	
	al_register_event_source(*eventos, al_get_keyboard_event_source());
	al_register_event_source(*eventos, al_get_display_event_source(*tela));
	al_register_event_source(*eventos, al_get_timer_event_source(*timer));
	al_register_event_source(*eventos, al_get_mouse_event_source());
	
	al_set_target_bitmap(al_get_backbuffer(*tela));
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_flip_display();
	al_start_timer(*timer);
	
	ALLEGRO_BITMAP *icon = al_load_bitmap("resources/applications-games.png");
	al_set_window_title(*tela, "Forever Alone Klondike <igorborges12@gmail.com>");
	al_set_display_icon(*tela, icon);
	
	return 0;
}

void fechar_allegro (ALLEGRO_DISPLAY **tela, ALLEGRO_EVENT_QUEUE **eventos, ALLEGRO_TIMER **timer) {
	//al_shutdown_ttf_addon();
	al_shutdown_image_addon();
	al_shutdown_font_addon();
	al_shutdown_primitives_addon();
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_timer(*timer);
	al_destroy_display(*tela);
	al_destroy_event_queue(*eventos);
	al_uninstall_system();
}

void sqlinject (int pts, char* nome) {
	char sql[100], *erro;
    int ret;
    
    sprintf(sql, "INSERT INTO score (nome, score) VALUES ('%s', %d)", nome, pts);
    ret = sqlite3_exec(db, sql, 0, 0, &erro);
    
    if (ret != SQLITE_OK) logger('e', "SQL: %s", erro);
    
    //CALL_SQLITE (prepare_v2 (db, sql, strlen(sql) + 1, &stmt, NULL));
    //CALL_SQLITE (bind_text (stmt, 1, "fruit", 6, SQLITE_STATIC));
    //CALL_SQLITE_EXPECT (step (stmt), DONE);
    //printf ("row id was %d\n", (int) sqlite3_last_insert_rowid (db));
    
    //sqlite3_close(db);
}

void sqltable () {
    char sql[100], *erro;
    int ret;
    
    sprintf(sql, "CREATE TABLE IF NOT EXISTS score (nome text, score int)");
    ret = sqlite3_exec(db, sql, 0, 0, &erro);
    
    if (ret != SQLITE_OK) logger('e', "SQL: %s", erro);
}

int counter = 0;

int select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names) {
	int i;
	int *p_rn = (int *) p_data;
	char fmt[100];
	
	(*p_rn)++;

	for (i = 0; i < num_fields; i++) {
		logger('i', "%s ", p_fields[i]);
		if (i < num_fields - 1) sprintf(fmt, "%-20s", p_fields[i]);
		else sprintf(fmt, "%s", p_fields[i]);
		strcat(high5[counter], fmt);
	}

	counter++;
	printf("\n");
	return 0;
}

void sqlselect() {
	char *erro;
	int ret;
	int nrecs = 0;

	counter = 0;
	
	for (ret = 0; ret < 5; ret++) sprintf(high5[ret], " ");

	ret = sqlite3_exec(db, "select * from score order by score desc limit 5", select_callback, &nrecs, &erro);

	if (ret != SQLITE_OK) logger('e', "SQL: %s", erro);
}
