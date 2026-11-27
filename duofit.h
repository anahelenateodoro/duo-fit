// DuoFit - arquivo com tudo que as fases usam em comum

#ifndef DUOFIT_H
#define DUOFIT_H

#define _CRT_SECURE_NO_WARNINGS   // Visual Studio: libera o uso do sprintf

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// tamanho da janela
#define LARGURA 1200
#define ALTURA  750

// o que cada fase devolve para o main saber o que abrir depois
#define VOLTAR_MENU  0
#define PROXIMA_FASE 1
#define FECHAR_JOGO  2

// pontuacao para ganhar o DuoFit
// cada fase vale no maximo 30 + 10 da pergunta = 40, e sao 3 fases = 120
#define PONTOS_MAXIMOS      120
#define PONTOS_PARA_GANHAR  96    // 80% de 120

// handebol e basquete: quantos arremessos o jogador tem para cumprir a meta
#define MAX_TENTATIVAS  7

// medidas usadas no handebol e no basquete (no Allegro o eixo Y cresce para BAIXO)
#define CHAO    630   // y onde fica o chao
#define RICO_L  140   // largura do Rico
#define RICO_A  210   // altura do Rico
#define BOLA_T  42    // a bola e um quadrado de 42 x 42

// Fontes, imagens e som sao carregados uma vez so no comeco do jogo
// e todas as fases usam os mesmos (assim gasta menos memoria).
// O "extern" quer dizer que a variavel foi criada em outro arquivo (comum.c).
extern ALLEGRO_FONT* fonte;          // texto normal
extern ALLEGRO_FONT* fonteGrande;    // titulos

extern ALLEGRO_FONT* fonteGiz;       // texto escrito na lousa
extern ALLEGRO_FONT* fonteGizTitulo; // titulos da lousa
extern ALLEGRO_FONT* fonteGizPequena; // textos menores do menu

// cores do giz
#define GIZ_BRANCO   al_map_rgb(238, 242, 232)
#define GIZ_AMARELO  al_map_rgb(250, 225, 120)
#define GIZ_VERDE    al_map_rgb(140, 240, 160)
#define GIZ_VERMELHO al_map_rgb(255, 140, 125)

extern ALLEGRO_BITMAP* bola;
extern ALLEGRO_BITMAP* anaParada;
extern ALLEGRO_BITMAP* anaAndando;
extern ALLEGRO_BITMAP* anaCorrendo;
extern ALLEGRO_BITMAP* anaPulando;
extern ALLEGRO_BITMAP* ricoParado;
extern ALLEGRO_BITMAP* ricoAndando;
extern ALLEGRO_BITMAP* ricoPulando;
extern ALLEGRO_BITMAP* fotoAna;     // fotos do menu
extern ALLEGRO_BITMAP* fotoRico;

extern int pontosTotal;   // soma dos pontos de todas as fases
extern int pontosPorFase[3];  // pontos de cada fase neste jogo (0 handebol, 1 atletismo, 2 basquete)
extern int recorde;           // maior pontuacao ja feita, fica salva no arquivo
extern int faseLiberada;  // ultima fase que o jogador pode abrir (1 handebol, 2 atletismo, 3 basquete)

// arquivo onde o recorde fica salvo
#define ARQUIVO_RECORDE "recorde.txt"

// comum.c
void carregar_recorde(void);
bool atualizar_recorde(int pontos);
bool clicou(ALLEGRO_EVENT evento);
void esperar_evento(ALLEGRO_EVENT_QUEUE* fila, ALLEGRO_EVENT* evento);
void clique_vira_enter(ALLEGRO_EVENT* evento);
bool carregar_recursos(void);
void liberar_recursos(void);
void escrever(ALLEGRO_FONT* f, ALLEGRO_COLOR cor, float y, char* mensagem);
void desenhar(ALLEGRO_BITMAP* imagem, float x, float y, float largura, float altura, int flags);
void tocar_som_gol(void);
ALLEGRO_EVENT_QUEUE* criar_fila(ALLEGRO_TIMER* timer);
int calcular_pontos(int erros, int perdaPorErro);
void desenhar_lousa(float x1, float y1, float x2, float y2);
void tela_regras(char* titulo, char* regras[5], char* meta, ALLEGRO_BITMAP* imagem, float largura, float altura);
void tela_fim(bool venceu, char* linha1, char* linha2);
void caixa_aviso(char* titulo, char* linha1, char* linha2);
void desenhar_barra(float porcento);
void desenhar_legenda(int quantidade, char* teclas[], char* acoes[]);
void atualizar_pulo(float* y, float* velY, bool* noChao);
void desenhar_rico(float x, float y, int direcao, bool noChao, int tempoAndando);
void desenhar_bola_na_mao(float x, float y, int direcao, bool noChao, int tempoDrible);
int verificar_saida(ALLEGRO_EVENT evento);
bool mover_rico(int tecla, int maxPassos, bool comBola, float* x, float* velY, int* direcao,
    bool* noChao, bool* jaMexeuNoAr, int* tempoAndando, int* passos, int* tempoDrible);

// pergunta.c
int fazer_pergunta(char* esporte, char* pergunta, char* opcoes[3], int correta,
    char* explicacao, int pontosFase, bool ultimaFase);

// uma funcao para cada tela
int mostrar_menu(void);
int jogar_handebol(void);
int jogar_atletismo(void);
int jogar_basquete(void);

#endif
