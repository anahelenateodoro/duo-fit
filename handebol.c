// =====================================================
// DuoFit - Fase de Handebol
// Projeto PI II - Ciencia da Computacao - Senac
// =====================================================

#define _CRT_SECURE_NO_WARNINGS   // Visual Studio: libera o uso do sprintf

#include <allegro5/allegro.h>             // base do Allegro (janela, eventos, timer)
#include <allegro5/allegro_primitives.h>  // desenhar retangulos e linhas
#include <allegro5/allegro_image.h>       // carregar imagens PNG
#include <allegro5/allegro_font.h>        // escrever texto
#include <allegro5/allegro_ttf.h>         // carregar fontes .ttf
#include <stdio.h>     // printf e sprintf
#include <stdlib.h>    // rand e srand
#include "rico_sprites.h"
#include "ana_sprites.h"
#include <stdbool.h>   // tipo bool (true / false)
#include <time.h>      // time, usado como semente do rand

// -----------------------------------------------------
// CONSTANTES
// #define troca o nome pelo valor antes de compilar
// -----------------------------------------------------

// estados do jogo: cada numero e uma tela diferente
#define MENU      0
#define REGRAS    1
#define HANDEBOL  2
#define VITORIA   3



// tamanho da janela em pixels
#define LARGURA 1200
#define ALTURA  750

#include "fases.h"
#include "navegacao.h"

// posicoes da quadra (no Allegro o eixo Y cresce para BAIXO)
#define CHAO    630    // y onde fica o chao
#define AREA    780    // x da linha dos 6 metros
#define GOL_X   1110   // x da trave do gol
#define GOL_Y   330    // y do travessao (parte de cima do gol)

// tamanhos dos desenhos
#define RICO_L      140   // largura do Rico
#define RICO_A      210   // altura do Rico
#define GOLEIRO_L  130
#define GOLEIRO_A  200
#define BOLA_T     42    // a bola e um quadrado de 42 x 42

// -----------------------------------------------------
// FUNCOES AUXILIARES
// -----------------------------------------------------

// Desenha a imagem na posicao (x, y) com a largura e altura que eu quiser.
// A imagem original pode ter outro tamanho: o Allegro estica ou encolhe.
// flags = 0 desenha normal, ALLEGRO_FLIP_HORIZONTAL desenha espelhado.
void desenhar(ALLEGRO_BITMAP* imagem, float x, float y, float largura, float altura, int flags) {
    float larguraOriginal = al_get_bitmap_width(imagem);
    float alturaOriginal = al_get_bitmap_height(imagem);

    al_draw_scaled_bitmap(imagem, 0, 0, larguraOriginal, alturaOriginal, x, y, largura, altura, flags);
}

// Escreve um texto centralizado na horizontal, na altura y.
void escrever(ALLEGRO_FONT* fonte, ALLEGRO_COLOR cor, float y, const char* mensagem) {
    al_draw_text(fonte, cor, LARGURA / 2, y, ALLEGRO_ALIGN_CENTER, mensagem);
}

// Verifica se dois retangulos estao se encostando (colisao).
// Retangulo 1: canto (x1, y1), tamanho l1 x a1
// Retangulo 2: canto (x2, y2), tamanho l2 x a2
// Eles se encostam quando se sobrepoem no eixo X E no eixo Y ao mesmo tempo.
bool colidiu(float x1, float y1, float l1, float a1, float x2, float y2, float l2, float a2) {
    bool sobrepoeX = (x1 < x2 + l2) && (x1 + l1 > x2);
    bool sobrepoeY = (y1 < y2 + a2) && (y1 + a1 > y2);
    return sobrepoeX && sobrepoeY;
}

// -----------------------------------------------------
// PROGRAMA PRINCIPAL
// -----------------------------------------------------

void jogar_handebol(void) {

    srand(time(NULL));   // muda a sequencia do rand a cada execucao

    // ---------- inicializar o Allegro ----------
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA, ALTURA);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();   // fila onde chegam teclas, timer etc.
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);       // dispara 60 vezes por segundo (60 FPS)

    ALLEGRO_FONT* fonte = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial.ttf", 24, 0);
    ALLEGRO_FONT* fonteGrande = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 56, 0);

    if (!janela || !fila || !timer) {
        fprintf(stderr, "Erro ao criar janela, fila ou timer.\n");
        return;
    }
    al_set_window_title(janela, "DuoFit");

    // diz para a fila "escutar" a janela, o teclado e o timer
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_mouse_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    // ---------- carregar imagens ----------
    // al_load_bitmap devolve um PONTEIRO para a imagem na memoria
    // se nao achar o arquivo, devolve NULL
    // Rico: jogador que arremessa
    ALLEGRO_BITMAP* atlasRico = carregar_atlas_rico();
    ALLEGRO_BITMAP* ricoParado = recortar_rico(atlasRico, RICO_PARADO);
    ALLEGRO_BITMAP* ricoAndando = recortar_rico(atlasRico, RICO_ANDANDO);
    ALLEGRO_BITMAP* ricoPulando = recortar_rico(atlasRico, RICO_PULANDO);
    // Ana: goleira
    ALLEGRO_BITMAP* atlasAna = carregar_atlas_ana();
    ALLEGRO_BITMAP* anaAndando = recortar_ana(atlasAna, ANA_ANDANDO1);

    ALLEGRO_BITMAP* anaPulando = recortar_ana(atlasAna, ANA_PULANDO);
    ALLEGRO_BITMAP* bola = al_load_bitmap("assets/bola.png");
    ALLEGRO_BITMAP* logo = al_load_bitmap("assets/logo.png");   // opcional

    // !ponteiro e verdadeiro quando o ponteiro e NULL (falhou)
    if (!fonte || !fonteGrande || !ricoParado || !ricoAndando ||
        !ricoPulando || !anaAndando || !anaPulando || !bola) {
        printf("Erro ao carregar fontes ou imagens.\n");
        return;
    }

    // ---------- cores (vermelho, verde, azul de 0 a 255) ----------
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR preto = al_map_rgb(0, 0, 0);
    ALLEGRO_COLOR amarelo = al_map_rgb(255, 220, 0);
    ALLEGRO_COLOR laranja = al_map_rgb(255, 140, 0);
    ALLEGRO_COLOR verde = al_map_rgb(0, 255, 120);
    ALLEGRO_COLOR azul = al_map_rgb(20, 45, 100);

    // ---------- variaveis do jogo ----------


    int estado = REGRAS;     // qual tela esta aparecendo


    // Rico: posicao (x, y) e velocidade vertical
    float ricoX = 150;
    float ricoY = CHAO - RICO_A;   // os pes do Rico ficam no chao
    float ricoVelY = 0;
    int direcao = 1;             // 1 = olhando para a direita, -1 = para a esquerda
    int tempoAndando = 0;        // enquanto for > 0, mostra a imagem "andando"
    bool noChao = true;
    bool jaMexeuNoAr = false;    // no ar ela so pode mudar de lugar 1 vez

    // bola
    bool comBola = true;         // true = bola na mao, false = bola voando
    int passos = 0;              // passos dados segurando a bola
    int tempoDrible = 0;         // enquanto for > 0, a bola esta quicando
    float bolaX = 0, bolaY = 0;
    float bolaVelX = 0, bolaVelY = 0;
    float forca = 0;             // barra de forca, vai de 0 a 100
    float variacaoForca = 2;     // +2 enchendo, -2 esvaziando

    // goleira (Ana)
    float goleiroX = 960;
    float goleiroY = CHAO - GOLEIRO_A;
    float goleiroVelX = -2;      // anda 2 pixels por frame
    float goleiroVelY = 0;       // diferente de 0 = esta pulando
    bool goleiroDecidiu = false; // ja sorteou se vai pular nesse arremesso?

    // placar e mensagens
    int gols = 0;
    int arremessos = 0;
    int infracao = 0;            // 0 = nenhuma, 1 = mais de 3 passos, 2 = pisou na area
    const char* resultado = NULL;  // ponteiro para o texto "GOOOL!" etc. NULL = sem mensagem
    int tempoResultado = 0;      // quantos frames a mensagem ainda fica na tela
    bool reiniciar = true;       // true = volta tudo para a posicao inicial

    bool rodando = true;
    bool redesenhar = false;

    al_start_timer(timer);

    // =================================================
    // LOOP PRINCIPAL
    // A cada volta: espera um evento, trata ele e desenha a tela
    // =================================================
    while (rodando) {

        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);
        if (pediu_voltar(&evento)) break;

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;   // clicou no X da janela
        }

        // =============================================
        // 1) TECLADO
        // =============================================
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {

            int tecla = evento.keyboard.keycode;

            // ----- menu -----


            // ----- tela de regras -----
            if (estado == REGRAS) {
                if (tecla == ALLEGRO_KEY_ENTER) {
                    // comeca a fase do zero
                    gols = 0;
                    arremessos = 0;
                    infracao = 0;
                    resultado = NULL;
                    reiniciar = true;
                    estado = HANDEBOL;
                }
                if (tecla == ALLEGRO_KEY_BACKSPACE) {
                    rodando = false;
                }
            }

            // ----- tela de vitoria -----
            else if (estado == VITORIA) {
                if (tecla == ALLEGRO_KEY_ENTER) {
                    rodando = false;
                }
            }

            // ----- fase de atletismo -----


            // ----- fase de handebol -----
            else if (estado == HANDEBOL) {

                if (tecla == ALLEGRO_KEY_BACKSPACE) {
                    rodando = false;
                }

                if (infracao != 0) {
                    // com o aviso aberto, so o ENTER funciona
                    if (tecla == ALLEGRO_KEY_ENTER) {
                        if (infracao == 2) {
                            reiniciar = true;   // pisou na area: volta para o comeco
                        }
                        infracao = 0;
                    }
                }
                else if (resultado == NULL) {
                    // so aceita comandos se nao tiver mensagem de gol/defesa na tela

                    // A = esquerda, D = direita
                    if (tecla == ALLEGRO_KEY_A || tecla == ALLEGRO_KEY_D) {

                        if (tecla == ALLEGRO_KEY_D) {
                            direcao = 1;
                        }
                        else {
                            direcao = -1;
                        }

                        if (!noChao) {
                            // no ar: nao conta passo, mas so pode mexer 1 vez
                            if (!jaMexeuNoAr) {
                                ricoX = ricoX + 60 * direcao;
                                jaMexeuNoAr = true;
                            }
                        }
                        else if (comBola && passos >= 3) {
                            infracao = 1;   // regra dos 3 passos
                        }
                        else {
                            ricoX = ricoX + 60 * direcao;   // direcao = -1 faz andar para tras
                            tempoAndando = 8;
                            if (comBola) {
                                passos++;
                            }
                        }
                    }

                    // W = pular (velocidade negativa = subir, porque o Y cresce para baixo)
                    if (tecla == ALLEGRO_KEY_W && noChao) {
                        ricoVelY = -16;
                        noChao = false;
                        jaMexeuNoAr = false;
                    }

                    // S = quicar a bola: zera os passos
                    if (tecla == ALLEGRO_KEY_S && comBola && noChao && tempoDrible == 0) {
                        tempoDrible = 20;
                        passos = 0;
                    }

                    // ESPACO = arremessar
                    if (tecla == ALLEGRO_KEY_SPACE && comBola && tempoDrible == 0) {

                        // a bola sai da mao do Rico
                        if (direcao == 1) {
                            bolaX = ricoX + 105;
                        }
                        else {
                            bolaX = ricoX - 5;
                        }
                        if (noChao) {
                            bolaY = ricoY + 100;
                        }
                        else {
                            bolaY = ricoY + 20;   // no pulo, a bola fica no punho levantado do Rico
                        }

                        bolaVelX = 18 * direcao;         // velocidade para o lado
                        bolaVelY = -1 - forca * 0.12;    // mais forca = sobe mais

                        comBola = false;
                        goleiroDecidiu = false;
                        arremessos++;
                    }
                }
            }
        }

        // =============================================
        // 2) ATUALIZAR O JOGO (a cada tique do timer)
        // =============================================
        if (evento.type == ALLEGRO_EVENT_TIMER) {

            redesenhar = true;



            if (estado == HANDEBOL) {

                // ----- volta tudo para o inicio -----
                if (reiniciar) {
                    ricoX = 150;
                    ricoY = CHAO - RICO_A;
                    ricoVelY = 0;
                    noChao = true;
                    direcao = 1;
                    comBola = true;
                    passos = 0;
                    tempoDrible = 0;
                    goleiroX = 960;
                    goleiroY = CHAO - GOLEIRO_A;
                    goleiroVelY = 0;
                    reiniciar = false;
                }

                // ----- fisica do pulo do Rico -----
                // a cada frame: posicao += velocidade  e  velocidade += gravidade
                // (e a ideia de derivada: a velocidade e a variacao da posicao,
                //  a gravidade e a variacao da velocidade -> o pulo vira uma parabola)
                ricoY = ricoY + ricoVelY;
                ricoVelY = ricoVelY + 0.8;

                if (ricoY >= CHAO - RICO_A) {   // encostou no chao
                    ricoY = CHAO - RICO_A;
                    ricoVelY = 0;
                    noChao = true;
                }

                // nao deixa sair da tela
                if (ricoX < 0) {
                    ricoX = 0;
                }
                if (ricoX > LARGURA - RICO_L) {
                    ricoX = LARGURA - RICO_L;
                }

                // contadores diminuem 1 por frame ate chegar em 0
                if (tempoAndando > 0) {
                    tempoAndando--;
                }
                if (tempoDrible > 0) {
                    tempoDrible--;
                }

                // ----- regra da area -----
                // o meio do corpo do Rico (ricoX + metade da largura) passou da linha
                if (comBola && noChao && infracao == 0 && ricoX + RICO_L / 2 > AREA) {
                    infracao = 2;
                }

                // ----- barra de forca: vai de 0 ate 100 e volta -----
                if (comBola) {
                    forca = forca + variacaoForca;
                    if (forca >= 100 || forca <= 0) {
                        variacaoForca = -variacaoForca;   // inverte o sentido
                    }
                }

                // ----- goleira anda de um lado para o outro -----
                goleiroX = goleiroX + goleiroVelX;
                if (goleiroX < 900 || goleiroX > 975) {
                    goleiroVelX = -goleiroVelX;
                }

                // ----- bola voando -----
                if (!comBola && resultado == NULL) {

                    // mesma fisica do pulo, so que com gravidade menor
                    bolaX = bolaX + bolaVelX;
                    bolaY = bolaY + bolaVelY;
                    bolaVelY = bolaVelY + 0.4;

                    // quica no chao perdendo 40% da velocidade
                    if (bolaY + BOLA_T >= CHAO) {
                        bolaY = CHAO - BOLA_T;
                        bolaVelY = -bolaVelY * 0.6;
                    }

                    // quando a bola chega perto, a goleira sorteia se pula
                    // rand() % 2 da 0 ou 1 -> 50% de chance
                    if (!goleiroDecidiu && bolaX > goleiroX - 280) {
                        goleiroDecidiu = true;
                        if (rand() % 2 == 1) {
                            goleiroVelY = -15;
                        }
                    }

                    // bateu na goleira? (uso uma caixa um pouco menor que a imagem)
                    if (colidiu(bolaX, bolaY, BOLA_T, BOLA_T,
                        goleiroX + 25, goleiroY + 20, GOLEIRO_L - 50, GOLEIRO_A - 20)) {
                        resultado = "DEFESA DA GOLEIRA!";
                    }
                    // chegou na linha do gol
                    else if (bolaX + BOLA_T >= GOL_X) {
                        if (bolaY >= GOL_Y) {        // passou por baixo do travessao
                            resultado = "GOOOL!";
                            gols++;
                        }
                        else {
                            resultado = "PARA FORA!";
                        }
                    }
                    // saiu pela esquerda da tela
                    else if (bolaX < -BOLA_T) {
                        resultado = "PARA FORA!";
                    }

                    if (resultado != NULL) {
                        tempoResultado = 90;   // 90 frames = 1,5 segundo
                    }
                }

                // ----- pulo da goleira (mesma fisica do Rico) -----
                if (goleiroVelY != 0) {
                    goleiroY = goleiroY + goleiroVelY;
                    goleiroVelY = goleiroVelY + 0.8;
                    if (goleiroY >= CHAO - GOLEIRO_A) {
                        goleiroY = CHAO - GOLEIRO_A;
                        goleiroVelY = 0;
                    }
                }

                // ----- tempo da mensagem de resultado -----
                if (resultado != NULL) {
                    tempoResultado--;
                    if (tempoResultado <= 0) {
                        resultado = NULL;
                        reiniciar = true;
                        if (gols >= 3) {
                            estado = VITORIA;
                        }
                    }
                }
            }
        }

        // =============================================
        // 3) DESENHAR
        // so desenha quando nao tem mais eventos esperando (evita travar)
        // =============================================
        if (redesenhar && al_is_event_queue_empty(fila)) {

            redesenhar = false;
            char texto[100];   // vetor de char para montar textos com sprintf

            // ----- menu -----




            // ----- regras -----
            if (estado == REGRAS) {
                al_clear_to_color(azul);
                escrever(fonteGrande, laranja, 50, "HANDEBOL");
                escrever(fonte, branco, 200, "1. Com a bola na mao, no maximo 3 passos.");
                escrever(fonte, branco, 250, "2. Aperte S para quicar a bola e ganhar mais 3 passos.");
                escrever(fonte, branco, 300, "3. Nao pise na area do goleiro segurando a bola.");
                escrever(fonte, amarelo, 350, "4. Mas pode pular na area e arremessar antes de cair!");
                escrever(fonte, branco, 400, "5. A barra de forca define a altura do arremesso.");
                escrever(fonte, verde, 470, "Faca 3 gols para passar de fase.");
                desenhar(bola, LARGURA / 2 - 40, 560, 80, 80, 0);
                escrever(fonte, branco, ALTURA - 60, "ENTER para comecar  |  ESC/BACKSPACE para o menu");
            }

            // ----- vitoria -----
            else if (estado == VITORIA) {
                al_clear_to_color(al_map_rgb(20, 100, 60));
                escrever(fonteGrande, branco, 150, "PARABENS!");
                sprintf(texto, "Gols: %d  |  Arremessos: %d", gols, arremessos);
                escrever(fonte, amarelo, 250, texto);
                desenhar(ricoPulando, LARGURA / 2 - 90, 320, 180, 270, 0);
                escrever(fonte, branco, ALTURA - 60, "ENTER para voltar ao menu");
            }

            // ----- fase de handebol -----
            else if (estado == HANDEBOL) {

                // fundo, chao azul e area laranja
                al_clear_to_color(al_map_rgb(215, 235, 245));
                al_draw_filled_rectangle(0, CHAO, AREA, ALTURA, al_map_rgb(40, 90, 170));
                al_draw_filled_rectangle(AREA, CHAO, LARGURA, ALTURA, al_map_rgb(240, 140, 40));
                al_draw_line(AREA, CHAO, AREA, CHAO + 45, branco, 5);

                // gol: trave (em pe) e travessao (deitado)
                al_draw_filled_rectangle(GOL_X - 8, GOL_Y, GOL_X, CHAO, al_map_rgb(220, 30, 30));
                al_draw_filled_rectangle(GOL_X - 8, GOL_Y - 8, LARGURA, GOL_Y, al_map_rgb(220, 30, 30));

                // placar
                sprintf(texto, "Gols: %d / 3    Passos: %d / 3", gols, passos);
                escrever(fonte, preto, 20, texto);

                // barra de forca: largura = forca * 2 (0 a 200 pixels)
                // a cor vai de verde (fraco) para vermelho (forte)
                al_draw_filled_rectangle(30, 30, 30 + forca * 2, 55, al_map_rgb(forca * 2.5, 200 - forca * 1.5, 40));
                al_draw_rectangle(30, 30, 230, 55, preto, 2);

                // goleira: escolhe a imagem certa
                if (goleiroVelY != 0) {
                    // a imagem do pulo olha para a direita, entao espelho
                    desenhar(anaPulando, goleiroX, goleiroY, GOLEIRO_L, GOLEIRO_A, ALLEGRO_FLIP_HORIZONTAL);
                }
                else if (goleiroVelX < 0) {
                    desenhar(anaAndando, goleiroX, goleiroY, GOLEIRO_L, GOLEIRO_A, ALLEGRO_FLIP_HORIZONTAL);
                }
                else {
                    desenhar(anaAndando, goleiroX, goleiroY, GOLEIRO_L, GOLEIRO_A, 0);
                }

                // Rico: escolhe a imagem certa
                // "imagem" e um ponteiro que vai apontar para uma das imagens carregadas
                ALLEGRO_BITMAP* imagem;
                int espelhar = 0;

                if (!noChao) {
                    imagem = ricoPulando;
                    if (direcao == -1) {
                        espelhar = ALLEGRO_FLIP_HORIZONTAL;
                    }
                }
                else if (tempoAndando > 0) {
                    if (direcao == 1) {
                        imagem = ricoAndando;
                    }
                    else {
                        imagem = ricoAndando;
                        espelhar = ALLEGRO_FLIP_HORIZONTAL;
                    }
                }
                else {
                    if (direcao == 1) {
                        imagem = ricoParado;
                    }
                    else {
                        imagem = ricoParado;
                        espelhar = ALLEGRO_FLIP_HORIZONTAL;
                    }
                }
                desenhar(imagem, ricoX, ricoY, RICO_L, RICO_A, espelhar);

                // bola na mao
                if (comBola) {
                    float maoX;
                    float maoY;

                    if (direcao == 1) {
                        maoX = ricoX + 102;
                    }
                    else {
                        maoX = ricoX - 3;
                    }
                    if (noChao) {
                        maoY = ricoY + 100;
                    }
                    else {
                        maoY = ricoY + 20;   // no pulo, a bola fica no punho levantado do Rico
                    }

                    // quicando: tempoDrible vai de 20 ate 0
                    // de 20 a 11 a bola desce, de 10 a 0 a bola sobe
                    if (tempoDrible > 0) {
                        float distanciaChao = (CHAO - BOLA_T) - maoY;
                        if (tempoDrible > 10) {
                            maoY = maoY + distanciaChao * (20 - tempoDrible) / 10.0;
                        }
                        else {
                            maoY = maoY + distanciaChao * tempoDrible / 10.0;
                        }
                    }

                    desenhar(bola, maoX, maoY, BOLA_T, BOLA_T, 0);
                }
                // bola voando
                else if (resultado == NULL) {
                    desenhar(bola, bolaX, bolaY, BOLA_T, BOLA_T, 0);
                }

                escrever(fonte, branco, ALTURA - 45, "A/D andar | W pular | S quicar | ESPACO arremessar | ESC/BACKSPACE menu");


                if (resultado != NULL) {
                    escrever(fonteGrande, laranja, 200, resultado);
                }

                // aviso de infracao
                if (infracao != 0) {
                    al_draw_filled_rectangle(280, 235, 920, 515, al_map_rgb(70, 20, 20));
                    al_draw_rectangle(280, 235, 920, 515, branco, 3);
                    escrever(fonte, al_map_rgb(255, 80, 80), 280, "INFRACAO!");

                    if (infracao == 1) {
                        escrever(fonte, branco, 335, "Mais de 3 passos segurando a bola.");
                        escrever(fonte, amarelo, 380, "Quique a bola (S) para andar mais.");
                    }
                    else {
                        escrever(fonte, branco, 335, "Voce pisou na area com a bola.");
                        escrever(fonte, amarelo, 380, "Pule e arremesse antes de cair!");
                    }
                    escrever(fonte, verde, 450, "ENTER para continuar");
                }
            }

            desenhar_voltar(fonte);
            al_flip_display();   
        }
    }

    al_destroy_bitmap(ricoParado);
    al_destroy_bitmap(ricoAndando);
    al_destroy_bitmap(ricoPulando);
    al_destroy_bitmap(atlasRico);
    al_destroy_bitmap(anaAndando);

    al_destroy_bitmap(anaPulando);
    al_destroy_bitmap(atlasAna);
    al_destroy_bitmap(bola);
    if (logo != NULL) {
        al_destroy_bitmap(logo);
    }
    al_destroy_font(fonte);
    al_destroy_font(fonteGrande);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);

    return;
}