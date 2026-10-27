#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include "fases.h"
#include "navegacao.h"
#include "rico_sprites.h"
#include "ana_sprites.h"

#define LARGURA 1200
#define ALTURA 750

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
static int frameCorrida;           // indice da pose no ciclo de animacao
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
            // Rico usa quatro poses: 60 a 100 ms por frame conforme a velocidade.
            float duracaoFrame = CORRIDA_TEMPO_FRAME;
            if (atletaAtual == 1) {
                float ritmo = velocidadeCorrida / CORRIDA_VELOCIDADE_MAX;
                duracaoFrame = 0.10f - 0.04f * ritmo;
            }
            while (tempoAnimacao >= duracaoFrame) {
                tempoAnimacao -= duracaoFrame;
                frameCorrida = (frameCorrida + 1) % 4;
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
    ALLEGRO_FONT *grande, ALLEGRO_BITMAP *parada,
    ALLEGRO_BITMAP *andando, ALLEGRO_BITMAP *andando2,
    ALLEGRO_BITMAP *anaSalto, ALLEGRO_BITMAP *ricoParado,
    ALLEGRO_BITMAP *ricoAndando, ALLEGRO_BITMAP *ricoSalto,
    ALLEGRO_BITMAP *ricoCorrida) {
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
        esperando = parada;
        parceiroX = CORRIDA_TROCA_INICIO;
        espelharParceiro = 0;
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

    ALLEGRO_BITMAP *sprite = parada;
    int flags = 0;
    if (velocidadeCorrida > 0) {
        // Alterna os recortes de caminhada e corrida da Ana.
        if (frameCorrida % 2 == 0) {
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
    float spriteX = posicaoCorredor - cameraCorrida - 70;
    if (atletaAtual == 1 && velocidadeCorrida > 0 && alturaSalto <= 0) {
        // A spritesheet tem espacamento irregular: cada pose possui seu recorte.
        // Mantem a mesma escala e ancora o torso e a sola, sem cortar os tenis.
        static const struct {
            int inicio, fim, pivoX, solaY;
        } frames[4] = {
            {32, 590, 353, 698},
            {600, 1025, 842, 699},
            {1030, 1625, 1391, 700},
            {1676, 2094, 1930, 700}
        };
        int indice = frameCorrida;
        float escalaFonte = al_get_bitmap_width(ricoCorrida) / 2172.0f;
        float escala = 210.0f / 724;
        float larguraFrame = frames[indice].fim - frames[indice].inicio;
        float desenhoX = spriteX + 70 -
            (frames[indice].pivoX - frames[indice].inicio) * escala;
        float desenhoY = 630 - frames[indice].solaY * escala;
        al_draw_scaled_bitmap(ricoCorrida, frames[indice].inicio * escalaFonte, 0,
            larguraFrame * escalaFonte, al_get_bitmap_height(ricoCorrida),
            desenhoX, desenhoY, larguraFrame * escala, 210, 0);
    } else {
        al_draw_scaled_bitmap(sprite, 0, 0, al_get_bitmap_width(sprite),
            al_get_bitmap_height(sprite), spriteX,
            420 - alturaSalto, 140, 210, flags);
    }
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
    if (!corridaTerminou && posicaoCorredor < barreirasX[0] && tempoAviso <= 0) {
        al_draw_text(fonte, escuro, 600, 220, ALLEGRO_ALIGN_CENTER,
            "Primeira barreira: observe a distancia e salte com ESPACO.");
    }
    if (tempoAviso > 0) {
        al_draw_text(fonte, escuro, 600, 220, ALLEGRO_ALIGN_CENTER,
            "Barreira derrubada! Salte um pouco antes com ESPACO.");
    }

    al_draw_text(fonte, escuro, LARGURA / 2, 20, ALLEGRO_ALIGN_CENTER,
        "DuoFit - Circuito educativo de atletismo");
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
        al_draw_filled_rounded_rectangle(260, 200, 940, 445, 16, 16, escuro);
        al_draw_text(grande, branco, 600, 220, ALLEGRO_ALIGN_CENTER, "CHEGOU!");
        al_draw_textf(fonte, branco, 600, 300, ALLEGRO_ALIGN_CENTER,
            "Ana + Rico: %.2f s | Barreiras derrubadas: %d", tempoCorrida, barreirasDerrubadas);
        al_draw_text(fonte, branco, 600, 345, ALLEGRO_ALIGN_CENTER,
            "Voce praticou ritmo, coordenacao e cooperacao!");
        al_draw_text(fonte, branco, 600, 380, ALLEGRO_ALIGN_CENTER,
            "Ana e Rico completaram juntos a passagem do bastao.");
        al_draw_text(fonte, branco, 600, 415, ALLEGRO_ALIGN_CENTER,
            "R para correr novamente | ESC para o menu");
    }
}

// As telas educativas usam o mesmo timer, mas nao iniciam o cronometro da prova.
static void desenhar_aula(ALLEGRO_FONT *fonte, ALLEGRO_BITMAP *ana,
    int tela, float salto, bool praticou, int resposta) {
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR azul = al_map_rgb(20, 45, 70);
    const char *titulos[] = {"Circuito educativo de atletismo", "Experimente o salto",
        "Revezamento: cooperar para chegar", "Desafio: voce aprendeu?"};
    const char *textos[][3] = {
        {"Este circuito combina fundamentos de barreiras e revezamento.",
         "Corra pela sua raia, salte as barreiras e leve o bastao ate a chegada.",
         "E uma atividade adaptada: nao representa uma unica prova oficial."},
        {"Observe a distancia: o momento do salto ajuda a manter o ritmo.",
         "Pressione ESPACO para experimentar. Aqui o tempo nao conta.",
         "No DuoFit, bater na barreira reduz a velocidade: uma adaptacao do jogo."},
        {"Ana comeca a corrida e Rico continua com o bastao.",
         "A passagem acontece dentro da zona de troca, destacada em amarelo.",
         "Neste jogo, pressione E no chao, dentro dessa zona."},
        {"Onde deve acontecer a passagem do bastao?",
         "A) Em qualquer ponto da pista.", "B) Dentro da zona de passagem."}
    };
    al_clear_to_color(al_map_rgb(225, 240, 248));
    al_draw_filled_rounded_rectangle(60, 35, 1140, 710, 18, 18, branco);
    al_draw_textf(fonte, azul, 600, 60, ALLEGRO_ALIGN_CENTER,
        "ATLETISMO | Aprender antes de jogar | %d / 4", tela + 1);
    al_draw_text(fonte, azul, 600, 115, ALLEGRO_ALIGN_CENTER, titulos[tela]);
    for (int i = 0; i < 3; i++)
        al_draw_text(fonte, azul, 600, 185 + i * 42, ALLEGRO_ALIGN_CENTER, textos[tela][i]);
    if (tela == 0 || tela == 1) {
        al_draw_filled_rectangle(180, 520, 1020, 570, al_map_rgb(190, 80, 55));
        al_draw_line(180, 550, 1020, 550, branco, 3);
        al_draw_scaled_bitmap(ana, 0, 0, al_get_bitmap_width(ana), al_get_bitmap_height(ana),
            420, 345 - salto, 120, 180, 0);
        al_draw_line(620, 525, 620, 450, azul, 5);
        al_draw_line(680, 525, 680, 450, azul, 5);
        al_draw_line(610, 450, 690, 450, al_map_rgb(230, 110, 35), 9);
    } else if (tela == 2) {
        al_draw_filled_rectangle(200, 390, 1000, 500, al_map_rgb(190, 80, 55));
        al_draw_filled_rectangle(450, 390, 750, 500, al_map_rgb(245, 190, 40));
        al_draw_text(fonte, azul, 325, 425, ALLEGRO_ALIGN_CENTER, "ANA  ->");
        al_draw_text(fonte, azul, 600, 425, ALLEGRO_ALIGN_CENTER, "BASTAO | E");
        al_draw_text(fonte, azul, 870, 425, ALLEGRO_ALIGN_CENTER, "->  RICO");
        al_draw_text(fonte, azul, 600, 525, ALLEGRO_ALIGN_CENTER, "Zona de troca: os dois atletas participam!");
    } else {
        al_draw_filled_rounded_rectangle(180, 340, 1020, 410, 12, 12, azul);
        al_draw_filled_rounded_rectangle(180, 430, 1020, 500, 12, 12, azul);
        al_draw_text(fonte, branco, 600, 360, ALLEGRO_ALIGN_CENTER, "A - Em qualquer ponto");
        al_draw_text(fonte, branco, 600, 450, ALLEGRO_ALIGN_CENTER, "B - Dentro da zona de passagem");
        if (resposta)
            al_draw_text(fonte, azul, 600, 540, ALLEGRO_ALIGN_CENTER,
                resposta == 2 ? "Isso! A zona organiza a passagem entre os atletas." :
                "Tente novamente: a passagem tem uma zona propria na pista.");
    }
    bool pode = (tela != 1 || praticou) && (tela != 3 || resposta == 2);
    al_draw_filled_rounded_rectangle(380, 600, 820, 655, 12, 12,
        pode ? al_map_rgb(30, 110, 130) : al_map_rgb(110, 120, 130));
    al_draw_text(fonte, branco, 600, 612, ALLEGRO_ALIGN_CENTER,
        !pode ? (tela == 1 ? "ESPACO para praticar" : "Escolha A ou B") :
        (tela == 3 ? "Comecar corrida | ENTER" : "Continuar | ENTER"));
    al_draw_text(fonte, azul, 600, 672, ALLEGRO_ALIGN_CENTER, "Clique nos botoes | ESC: voltar ao menu");
}

void jogar_atletismo(void) {
    ALLEGRO_DISPLAY *janela = al_create_display(LARGURA, ALTURA);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_FONT *fonte = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial.ttf", 24, 0);
    ALLEGRO_FONT *grande = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial Bold.ttf", 56, 0);
    ALLEGRO_BITMAP *atlasRico = carregar_atlas_rico();
    ALLEGRO_BITMAP *atlasAna = carregar_atlas_ana();
    ALLEGRO_BITMAP *imagens[8];
    const int posesAna[4] = {ANA_PARADA, ANA_ANDANDO1, ANA_CORRENDO2, ANA_PULANDO};
    bool carregou = janela && fila && timer && fonte && grande;
    for (int i = 0; i < 8; i++) {
        imagens[i] = i < 4 ? recortar_ana(atlasAna, posesAna[i]) : recortar_rico(atlasRico, i - 4);
        if (!imagens[i]) carregou = false;
    }
    if (carregou) {
        al_set_window_title(janela, "DuoFit - Atletismo");
        al_register_event_source(fila, al_get_display_event_source(janela));
        al_register_event_source(fila, al_get_keyboard_event_source());
        al_register_event_source(fila, al_get_timer_event_source(timer));
        al_register_event_source(fila, al_get_mouse_event_source());
        int telaAula = 0;
        int resposta = 0;
        float saltoTreino = 0, velocidadeTreino = 0;
        bool praticou = false;
        iniciar_corrida();
        bool rodando = true;
        bool redesenhar = true;
        al_start_timer(timer);
        while (rodando) {
            ALLEGRO_EVENT evento;
            al_wait_for_event(fila, &evento);
            if (pediu_voltar(&evento)) break;
            if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) rodando = false;
            if (evento.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
                teclaDCorrida = false;
                teclaDireitaCorrida = false;
            }
            if (telaAula < 4) {
                bool tecla = evento.type == ALLEGRO_EVENT_KEY_DOWN;
                int codigo = tecla ? evento.keyboard.keycode : 0;
                bool clique = evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && evento.mouse.button == 1;
                if (tecla && (codigo == ALLEGRO_KEY_ESCAPE || codigo == ALLEGRO_KEY_BACKSPACE))
                    rodando = false;
                if (telaAula == 1 && tecla && codigo == ALLEGRO_KEY_SPACE && saltoTreino == 0) {
                    velocidadeTreino = 650;
                    praticou = true;
                }
                if (telaAula == 3) {
                    if (codigo == ALLEGRO_KEY_A || (clique && evento.mouse.x >= 180 &&
                        evento.mouse.x <= 1020 && evento.mouse.y >= 340 && evento.mouse.y <= 410)) resposta = 1;
                    if (codigo == ALLEGRO_KEY_B || (clique && evento.mouse.x >= 180 &&
                        evento.mouse.x <= 1020 && evento.mouse.y >= 430 && evento.mouse.y <= 500)) resposta = 2;
                }
                bool continuar = codigo == ALLEGRO_KEY_ENTER || (clique && evento.mouse.x >= 380 &&
                    evento.mouse.x <= 820 && evento.mouse.y >= 600 && evento.mouse.y <= 655);
                if (continuar && (telaAula != 1 || praticou) && (telaAula != 3 || resposta == 2)) {
                    telaAula++;
                    if (telaAula == 4) iniciar_corrida();
                }
                if (evento.type == ALLEGRO_EVENT_TIMER) {
                    if (saltoTreino > 0 || velocidadeTreino > 0) {
                        saltoTreino += velocidadeTreino / 60;
                        velocidadeTreino -= 1600.0f / 60;
                        if (saltoTreino < 0) { saltoTreino = 0; velocidadeTreino = 0; }
                    }
                }
                if (rodando) {
                    if (telaAula < 4)
                        desenhar_aula(fonte, saltoTreino > 0 ? imagens[3] : imagens[1],
                            telaAula, saltoTreino, praticou, resposta);
                    else
                        desenhar_corrida(fonte, grande, imagens[0], imagens[1], imagens[2],
                            imagens[3], imagens[4], imagens[5], imagens[6], imagens[7]);
                    desenhar_voltar(fonte);
                    al_flip_display();
                }
                continue;
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
                    imagens[3], imagens[4], imagens[5], imagens[6], imagens[7]);
                desenhar_voltar(fonte);
                al_flip_display();
                redesenhar = false;
            }
        }
    } else {
        printf("Erro ao carregar a fase de atletismo.\n");
    }
    for (int i = 0; i < 8; i++) {
        if (imagens[i]) al_destroy_bitmap(imagens[i]);
    }
    if (atlasRico) al_destroy_bitmap(atlasRico);
    if (atlasAna) al_destroy_bitmap(atlasAna);
    if (fonte) al_destroy_font(fonte);
    if (grande) al_destroy_font(grande);
    if (timer) al_destroy_timer(timer);
    if (fila) al_destroy_event_queue(fila);
    if (janela) al_destroy_display(janela);
}
