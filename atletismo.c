#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include "fases.h"

#define LARGURA 1200
#define ALTURA 750

// Sprint 1: valores em pixels e segundos, separados do handebol.
#define CORRIDA_INICIO 150.0f
#define CORRIDA_CHEGADA 4150.0f
#define CORRIDA_MUNDO 4500.0f
#define CORRIDA_VELOCIDADE_MAX 520.0f
#define CORRIDA_ACELERACAO 260.0f
#define CORRIDA_DESACELERACAO 340.0f
#define CORRIDA_TEMPO_FRAME 0.12f

#define CORRIDA_TROCA_INICIO 1950.0f
#define CORRIDA_TROCA_FIM 2350.0f
#define CORRIDA_TROCA_X 2150.0f
#define CORRIDA_BARREIRAS 6
static const float barreirasX[CORRIDA_BARREIRAS] = {700, 1250, 1750, 2700, 3250, 3800};

// Variaveis da fase. As funcoes abaixo usam esses valores diretamente.
// O prefixo static mantem essas variaveis dentro deste arquivo do jogo.
static float posicaoCorredor;       // posicao na pista
static float velocidadeCorrida;    // pixels por segundo
static float cameraCorrida;        // deslocamento do cenario
static float tempoCorrida;         // cronometro
static float tempoAnimacao;        // tempo desde a troca de imagem
static int frameCorrida;           // imagem 0 ou imagem 1
static int atletaAtual;            // 0 = Ana, 1 = Rico
static int barreirasDerrubadas;
static float alturaSalto;          // altura dos pes acima do chao
static float velocidadeSalto;
static float tempoAviso;
static bool derrubadas[CORRIDA_BARREIRAS];
static bool teclaDCorrida;
static bool teclaDireitaCorrida;
static bool corridaTerminou;

static void iniciar_corrida(void) {
    posicaoCorredor = CORRIDA_INICIO;
    velocidadeCorrida = 0;
    cameraCorrida = 0;
    tempoCorrida = 0;
    tempoAnimacao = 0;
    frameCorrida = 0;
    atletaAtual = 0;
    barreirasDerrubadas = 0;
    alturaSalto = 0;
    velocidadeSalto = 0;
    tempoAviso = 0;
    teclaDCorrida = false;
    teclaDireitaCorrida = false;
    corridaTerminou = false;
    for (int i = 0; i < CORRIDA_BARREIRAS; i++) {
        derrubadas[i] = false;
    }
}

// Salto so pode comecar no chao; segurar a tecla nao repete o salto.
static void saltar_corrida(void) {
    if (!corridaTerminou && alturaSalto == 0) {
        velocidadeSalto = 650;
    }
}

static void passar_bastao(void) {
    if (!corridaTerminou && atletaAtual == 0 && alturaSalto == 0 &&
        posicaoCorredor >= CORRIDA_TROCA_INICIO && posicaoCorredor <= CORRIDA_TROCA_FIM) {
        atletaAtual = 1;
        posicaoCorredor = CORRIDA_TROCA_X;
        tempoAnimacao = 0;
        frameCorrida = 0;
    }
}

static void atualizar_corrida(float dt) {
    if (corridaTerminou) {
        return;
    }
    float xAnterior = posicaoCorredor;
    if (tempoAviso > 0) {
        tempoAviso = tempoAviso - (dt);
    }
    alturaSalto = alturaSalto + (velocidadeSalto * dt);
    if (alturaSalto > 0 || velocidadeSalto > 0) {
        velocidadeSalto = velocidadeSalto - (1600 * dt);
    }
    if (alturaSalto < 0) {
        alturaSalto = 0;
        velocidadeSalto = 0;
    }
    // Dois controles independentes: soltar um nao cancela o outro.
    float anterior = velocidadeCorrida;
    if (teclaDCorrida || teclaDireitaCorrida) {
        velocidadeCorrida = velocidadeCorrida + (CORRIDA_ACELERACAO * dt);
        if (velocidadeCorrida > CORRIDA_VELOCIDADE_MAX) {
            velocidadeCorrida = CORRIDA_VELOCIDADE_MAX;
        }
    } else {
        velocidadeCorrida = velocidadeCorrida - (CORRIDA_DESACELERACAO * dt);
        if (velocidadeCorrida < 0) {
            velocidadeCorrida = 0;
        }
    }
    // dt e o tempo de uma atualizacao: no jogo, 1/60 de segundo.
    // Usamos a media entre a velocidade antes e depois da aceleracao.
    float velocidadeMedia = (anterior + velocidadeCorrida) / 2;
    float deslocamento = velocidadeMedia * dt;
    if (deslocamento > 0) {
        float restante = CORRIDA_CHEGADA - posicaoCorredor;
        if (deslocamento >= restante) {
            tempoCorrida = tempoCorrida + (dt * restante / deslocamento);
            posicaoCorredor = CORRIDA_CHEGADA;
            velocidadeCorrida = 0;
            corridaTerminou = true;
        } else {
            posicaoCorredor = posicaoCorredor + (deslocamento);
            tempoCorrida = tempoCorrida + (dt);
        }
        // O timer troca a pose continuamente, mesmo sem novos eventos de tecla.
        if (velocidadeCorrida > 0) {
            tempoAnimacao = tempoAnimacao + (dt);
            while (tempoAnimacao >= CORRIDA_TEMPO_FRAME) {
                tempoAnimacao = tempoAnimacao - (CORRIDA_TEMPO_FRAME);
                if (frameCorrida == 0) {
                    frameCorrida = 1;
                } else {
                    frameCorrida = 0;
                }
            }
        }
    } else if (posicaoCorredor > CORRIDA_INICIO) {
        tempoCorrida = tempoCorrida + (dt);
    }
    // Ana espera a troca no fim da zona: o percurso exige os dois atletas.
    if (atletaAtual == 0 && posicaoCorredor >= CORRIDA_TROCA_FIM) {
        posicaoCorredor = CORRIDA_TROCA_FIM;
        velocidadeCorrida = 0;
    }
    for (int i = 0; i < CORRIDA_BARREIRAS; i++) {
        // Teste do intervalo percorrido evita atravessar a barreira entre frames.
        if (!derrubadas[i] && xAnterior <= barreirasX[i] + 22 &&
            posicaoCorredor >= barreirasX[i] - 22 && alturaSalto < 85) {
            derrubadas[i] = true;
            barreirasDerrubadas = barreirasDerrubadas + 1;
            velocidadeCorrida = velocidadeCorrida * (0.4f);
            tempoAviso = 1.5f;
        }
    }
    if (velocidadeCorrida == 0) {
        tempoAnimacao = 0;
        frameCorrida = 0;
    }
    // Camera segue Ana depois do primeiro trecho e para no fim do mundo.
    cameraCorrida = posicaoCorredor - 350;
    if (cameraCorrida < 0) {
        cameraCorrida = 0;
    }
    if (cameraCorrida > CORRIDA_MUNDO - LARGURA) {
        cameraCorrida = CORRIDA_MUNDO - LARGURA;
    }
}

static void desenhar_corrida(ALLEGRO_FONT *fonte,
    ALLEGRO_FONT *grande, ALLEGRO_BITMAP *paradaEsq,
    ALLEGRO_BITMAP *andando, ALLEGRO_BITMAP *andando2,
    ALLEGRO_BITMAP *anaSalto, ALLEGRO_BITMAP *ricoParado,
    ALLEGRO_BITMAP *ricoAndando, ALLEGRO_BITMAP *ricoSalto) {
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR escuro = al_map_rgb(20, 45, 70);
    al_clear_to_color(al_map_rgb(185, 225, 245));
    al_draw_filled_circle(1050, 170, 45, al_map_rgb(255, 220, 100));
    // Arquibancada com deslocamento menor: profundidade no cenario.
    for (int i = 0; i < 20; i++) {
        float x = i * 280 - cameraCorrida * 0.25f;
        al_draw_filled_rectangle(x, 330, x + 240, 440, al_map_rgb(90, 130, 150));
        for (int j = 0; j < 3; j++) {
            al_draw_line(x, 355 + j * 30, x + 240, 355 + j * 30, branco, 3);
        }
    }
    al_draw_filled_rectangle(0, 440, LARGURA, ALTURA, al_map_rgb(70, 155, 85));
    al_draw_filled_rectangle(0, 530, LARGURA, 690, al_map_rgb(190, 80, 55));
    for (int i = 0; i <= 4; i++) {
        al_draw_line(0, 530 + i * 40, LARGURA, 530 + i * 40, branco, 3);
    }
    for (int i = 0; i <= 100; i += 10) {
        float x = CORRIDA_INICIO + i * 40 - cameraCorrida;
        if (x < -80 || x > LARGURA + 80) {
            continue;
        }
        al_draw_line(x, 535, x, 685, branco, 2);
        al_draw_textf(fonte, branco, x + 5, 650, 0, "%d m", i);
    }
    float chegada = CORRIDA_CHEGADA - cameraCorrida;
    // Quadrados alternados formam a faixa de chegada.
    for (int y = 530; y < 690; y = y + 20) {
        for (int j = 0; j < 2; j++) {
            ALLEGRO_COLOR corQuadrado;
            if ((y / 20 + j) % 2 == 0) {
                corQuadrado = escuro;
            } else {
                corQuadrado = branco;
            }
            al_draw_filled_rectangle(chegada + j * 20, y,
                chegada + (j + 1) * 20, y + 20, corQuadrado);
        }
    }
    if (chegada <= LARGURA) {
        al_draw_text(fonte, escuro, chegada, 490, ALLEGRO_ALIGN_CENTER, "CHEGADA");
    }

    float zona = CORRIDA_TROCA_INICIO - cameraCorrida;
    al_draw_filled_rectangle(zona, 590, CORRIDA_TROCA_FIM - cameraCorrida, 630,
        al_map_rgba(245, 190, 40, 255));
    al_draw_text(fonte, escuro, CORRIDA_TROCA_X - cameraCorrida, 390,
        ALLEGRO_ALIGN_CENTER, "ZONA DE TROCA - E");
    // O parceiro fica visivel esperando ou apos terminar sua parte.
    ALLEGRO_BITMAP *esperando;
    float parceiroX;
    int espelharParceiro;
    if (atletaAtual == 0) {
        esperando = ricoParado;
        parceiroX = CORRIDA_TROCA_X;
        espelharParceiro = 0;
    } else {
        esperando = paradaEsq;
        parceiroX = CORRIDA_TROCA_INICIO;
        espelharParceiro = ALLEGRO_FLIP_HORIZONTAL;
    }
    al_draw_scaled_bitmap(esperando, 0, 0, al_get_bitmap_width(esperando),
        al_get_bitmap_height(esperando), parceiroX - cameraCorrida - 70,
        380, 140, 210, espelharParceiro);
    for (int i = 0; i < CORRIDA_BARREIRAS; i++) {
        float x = barreirasX[i] - cameraCorrida;
        if (x < -60 || x > LARGURA + 60) {
            continue;
        }
        if (derrubadas[i]) {
            al_draw_filled_rectangle(x - 35, 622, x + 35, 630, branco);
        } else {
            al_draw_line(x - 25, 630, x - 25, 550, escuro, 5);
            al_draw_line(x + 25, 630, x + 25, 550, escuro, 5);
            al_draw_filled_rectangle(x - 32, 542, x + 32, 553, branco);
            for (int j = 0; j < 4; j++) {
                al_draw_filled_rectangle(x - 32 + j * 16, 542,
                    x - 24 + j * 16, 553, al_map_rgb(220, 60, 50));
            }
        }
    }

    ALLEGRO_BITMAP *sprite = paradaEsq;
    int flags = ALLEGRO_FLIP_HORIZONTAL;
    if (velocidadeCorrida > 0) {
        // Alterna diretamente ana_andando1.png e ana_correndo2.png.
        if (frameCorrida == 0) {
            sprite = andando;
        } else {
            sprite = andando2;
        }
        flags = 0;
    }
    if (atletaAtual == 1) {
        if (velocidadeCorrida > 0) {
            sprite = ricoAndando;
        } else {
            sprite = ricoParado;
        }
        flags = 0;
    }
    if (alturaSalto > 0) {
        if (atletaAtual == 0) {
            sprite = anaSalto;
        } else {
            sprite = ricoSalto;
        }
        flags = 0;
    }
    al_draw_scaled_bitmap(sprite, 0, 0, al_get_bitmap_width(sprite),
        al_get_bitmap_height(sprite), posicaoCorredor - cameraCorrida - 70,
        420 - alturaSalto, 140, 210, flags);
    // Bastao acompanha a mao do atleta que esta correndo.
    float maoX = posicaoCorredor - cameraCorrida + 35;
    al_draw_line(maoX, 520 - alturaSalto, maoX + 25, 505 - alturaSalto,
        al_map_rgb(255, 215, 20), 7);
    const char *nomeAtleta;
    if (atletaAtual == 0) {
        nomeAtleta = "Ana";
    } else {
        nomeAtleta = "Rico";
    }
    al_draw_textf(fonte, escuro, 600, 145, ALLEGRO_ALIGN_CENTER,
        "Correndo: %s | Barreiras derrubadas: %d", nomeAtleta, barreirasDerrubadas);
    if (atletaAtual == 0 && posicaoCorredor >= CORRIDA_TROCA_INICIO) {
        al_draw_text(fonte, escuro, 600, 180, ALLEGRO_ALIGN_CENTER,
            "Passe o bastao ao Rico: pressione E no chao!");
    }
    if (tempoAviso > 0) {
        al_draw_text(fonte, escuro, 600, 220, ALLEGRO_ALIGN_CENTER,
            "Barreira derrubada! Salte um pouco antes com ESPACO.");
    }

    al_draw_text(fonte, escuro, LARGURA / 2, 20, ALLEGRO_ALIGN_CENTER,
        "DuoFit - Fase 2: Atletismo");
    float metros = (posicaoCorredor - CORRIDA_INICIO) / 40;
    al_draw_textf(fonte, escuro, LARGURA / 2, 60, ALLEGRO_ALIGN_CENTER,
        "Percurso: %.1f / 100 m | Velocidade: %.1f m/s | Tempo: %.2f s",
        metros, velocidadeCorrida / 40, tempoCorrida);
    al_draw_filled_rectangle(250, 110, 950, 126, al_map_rgb(130, 170, 185));
    al_draw_filled_rectangle(250, 110, 250 + 700 * metros / 100, 126,
        al_map_rgb(245, 150, 40));
    al_draw_text(fonte, branco, LARGURA / 2, 710, ALLEGRO_ALIGN_CENTER,
        "D / seta: correr | ESPACO: saltar | E: bastao | R: reiniciar | ESC: menu");
    if (corridaTerminou) {
        al_draw_filled_rounded_rectangle(260, 200, 940, 385, 16, 16, escuro);
        al_draw_text(grande, branco, 600, 220, ALLEGRO_ALIGN_CENTER, "CHEGOU!");
        al_draw_textf(fonte, branco, 600, 300, ALLEGRO_ALIGN_CENTER,
            "Ana + Rico: %.2f s | Barreiras derrubadas: %d", tempoCorrida, barreirasDerrubadas);
        al_draw_text(fonte, branco, 600, 345, ALLEGRO_ALIGN_CENTER,
            "R para correr novamente | ESC para o menu");
    }
}

// Abre a fase, recebe as teclas e libera os recursos ao sair.
void jogar_atletismo(void) {
    ALLEGRO_DISPLAY *janela = al_create_display(LARGURA, ALTURA);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_FONT *fonte = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial.ttf", 24, 0);
    ALLEGRO_FONT *grande = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 56, 0);
    ALLEGRO_BITMAP *imagens[7];
    const char *arquivos[7] = {
        "assets/ana_paradaesquerda.png", "assets/ana_andando1.png",
        "assets/ana_correndo2.png", "assets/ana_pulando1.png",
        "assets/rico_parado.png", "assets/rico_andando.png", "assets/rico_pulando.png"
    };
    bool carregou = janela && fila && timer && fonte && grande;
    for (int i = 0; i < 7; i++) {
        imagens[i] = al_load_bitmap(arquivos[i]);
        if (!imagens[i]) carregou = false;
    }
    if (carregou) {
        al_set_window_title(janela, "DuoFit - Atletismo");
        al_register_event_source(fila, al_get_display_event_source(janela));
        al_register_event_source(fila, al_get_keyboard_event_source());
        al_register_event_source(fila, al_get_timer_event_source(timer));
        iniciar_corrida();
        bool rodando = true;
        bool redesenhar = true;
        al_start_timer(timer);
        while (rodando) {
            ALLEGRO_EVENT evento;
            al_wait_for_event(fila, &evento);
            if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) rodando = false;
            if (evento.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
                teclaDCorrida = false;
                teclaDireitaCorrida = false;
            }
            if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
                int tecla = evento.keyboard.keycode;
                if (tecla == ALLEGRO_KEY_ESCAPE || tecla == ALLEGRO_KEY_BACKSPACE) rodando = false;
                else if (tecla == ALLEGRO_KEY_D) teclaDCorrida = true;
                else if (tecla == ALLEGRO_KEY_RIGHT) teclaDireitaCorrida = true;
                else if (tecla == ALLEGRO_KEY_SPACE) saltar_corrida();
                else if (tecla == ALLEGRO_KEY_E) passar_bastao();
                else if (tecla == ALLEGRO_KEY_R) iniciar_corrida();
            }
            if (evento.type == ALLEGRO_EVENT_KEY_UP) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_D) teclaDCorrida = false;
                if (evento.keyboard.keycode == ALLEGRO_KEY_RIGHT) teclaDireitaCorrida = false;
            }
            if (evento.type == ALLEGRO_EVENT_TIMER) {
                atualizar_corrida(1.0f / 60.0f);
                redesenhar = true;
            }
            if (rodando && redesenhar && al_is_event_queue_empty(fila)) {
                desenhar_corrida(fonte, grande, imagens[0], imagens[1], imagens[2],
                    imagens[3], imagens[4], imagens[5], imagens[6]);
                al_flip_display();
                redesenhar = false;
            }
        }
    } else {
        printf("Erro ao carregar a fase de atletismo.\n");
    }
    for (int i = 0; i < 7; i++) {
        if (imagens[i]) al_destroy_bitmap(imagens[i]);
    }
    if (fonte) al_destroy_font(fonte);
    if (grande) al_destroy_font(grande);
    if (timer) al_destroy_timer(timer);
    if (fila) al_destroy_event_queue(fila);
    if (janela) al_destroy_display(janela);
}
