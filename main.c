#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <stdio.h>
#include <stdbool.h>

int main() {

    // -------------------------
    // INICIALIZACAO
    // -------------------------

    if (!al_init()) {
        printf("Erro ao iniciar Allegro.\n");
        return 1;
    }

    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    int largura = 1200;
    int altura = 750;

    ALLEGRO_DISPLAY *display = al_create_display(largura, altura);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

    ALLEGRO_FONT *fonte = al_load_ttf_font(
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        24,
        0
    );

    if (!display || !fila || !timer || !fonte) {
        printf("Erro ao iniciar o jogo.\n");
        return 1;
    }

    al_set_window_title(display, "DuoFit");

    al_register_event_source(fila, al_get_display_event_source(display));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    // -------------------------
    // CARREGAR SPRITES
    // -------------------------

    ALLEGRO_BITMAP *anaParadaDireita =
        al_load_bitmap("assets/ana_parada1.png");

    ALLEGRO_BITMAP *anaParadaEsquerda =
        al_load_bitmap("assets/ana_paradaesquerda.png");

    ALLEGRO_BITMAP *anaAndandoDireita1 =
        al_load_bitmap("assets/ana_andando1.png");

    ALLEGRO_BITMAP *anaAndandoDireita2 =
        al_load_bitmap("assets/ana_andando2.png");

    ALLEGRO_BITMAP *anaAndandoEsquerda1 =
        al_load_bitmap("assets/ana_andandoesquerda1.png");

    ALLEGRO_BITMAP *anaPulando =
        al_load_bitmap("assets/ana_pulando1.png");

    ALLEGRO_BITMAP *bola =
        al_load_bitmap("assets/bola.png");

    if (!anaParadaDireita ||
        !anaParadaEsquerda ||
        !anaAndandoDireita1 ||
        !anaAndandoDireita2 ||
        !anaAndandoEsquerda1 ||
        !anaPulando ||
        !bola) {

        printf("Erro ao carregar imagens.\n");
        return 1;
    }

    // -------------------------
    // POSICAO DA ANA
    // -------------------------

    float larguraAna = 140;
    float alturaAna = 210;

    float chaoY = altura - 120;

    float anaX = 150;
    float anaY = chaoY - alturaAna;

    // -------------------------
    // MOVIMENTO / PULO
    // -------------------------

    float velocidadeY = 0;
    float gravidade = 0.8;
    float forcaPulo = -16;

    bool noChao = true;
    bool pulando = false;

    // 1 = direita
    // -1 = esquerda
    int ultimaDirecao = 1;

    // -------------------------
    // REGRA DOS 3 PASSOS
    // -------------------------

    bool estaComBola = true;
    int passosComBola = 0;
    bool mostrarInfracao = false;

    // -------------------------
    // ANIMACAO
    // -------------------------

    bool andando = false;

    int frameAndando = 0;
    int contadorAnimacao = 0;

    // -------------------------
    // CONTROLE
    // -------------------------

    bool rodando = true;
    bool redesenhar = true;

    al_start_timer(timer);

    // -------------------------
    // GAME LOOP
    // -------------------------

    while (rodando) {

        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        // fechar janela
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }

        // -------------------------
        // TECLAS
        // -------------------------

        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {

            if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                rodando = false;
            }

            // fecha popup
            if (
                evento.keyboard.keycode == ALLEGRO_KEY_ENTER &&
                mostrarInfracao
            ) {
                mostrarInfracao = false;
                continue;
            }

            if (mostrarInfracao) {
                continue;
            }

            // -------------------------
            // A = ESQUERDA
            // -------------------------

            if (evento.keyboard.keycode == ALLEGRO_KEY_A) {

                ultimaDirecao = -1;

                if (estaComBola) {

                    if (passosComBola < 3) {

                        anaX -= 60;
                        passosComBola++;
                        andando = true;

                    } else {

                        mostrarInfracao = true;
                    }

                } else {

                    anaX -= 60;
                    andando = true;
                }
            }

            // -------------------------
            // D = DIREITA
            // -------------------------

            if (evento.keyboard.keycode == ALLEGRO_KEY_D) {

                ultimaDirecao = 1;

                if (estaComBola) {

                    if (passosComBola < 3) {

                        anaX += 60;
                        passosComBola++;
                        andando = true;

                    } else {

                        mostrarInfracao = true;
                    }

                } else {

                    anaX += 60;
                    andando = true;
                }
            }

            // -------------------------
            // W = PULAR
            // -------------------------

            if (
                evento.keyboard.keycode == ALLEGRO_KEY_W &&
                noChao
            ) {

                velocidadeY = forcaPulo;
                noChao = false;
                pulando = true;
            }
        }

        // -------------------------
        // TIMER
        // -------------------------

        if (evento.type == ALLEGRO_EVENT_TIMER) {

            // gravidade
            velocidadeY += gravidade;
            anaY += velocidadeY;

            // chão
            float posicaoChaoAna = chaoY - alturaAna;

            if (anaY >= posicaoChaoAna) {

                anaY = posicaoChaoAna;
                velocidadeY = 0;
                noChao = true;
                pulando = false;
            }

            // limites
            if (anaX < 0) {
                anaX = 0;
            }

            if (anaX > largura - larguraAna) {
                anaX = largura - larguraAna;
            }

            // animação andando
            if (andando && !pulando) {

                contadorAnimacao++;

                if (contadorAnimacao >= 8) {

                    frameAndando++;

                    if (frameAndando > 1) {
                        frameAndando = 0;
                    }

                    contadorAnimacao = 0;
                    andando = false;
                }
            }

            redesenhar = true;
        }

        // -------------------------
        // DESENHAR
        // -------------------------

        if (
            redesenhar &&
            al_is_event_queue_empty(fila)
        ) {

            redesenhar = false;

            // fundo
            al_clear_to_color(
                al_map_rgb(215, 235, 245)
            );

            // -------------------------
            // CHAO
            // -------------------------

            al_draw_filled_rectangle(
                0,
                chaoY,
                largura,
                altura,
                al_map_rgb(60, 160, 80)
            );

            al_draw_line(
                0,
                chaoY,
                largura,
                chaoY,
                al_map_rgb(255, 255, 255),
                4
            );

            // -------------------------
            // TITULO
            // -------------------------

            al_draw_text(
                fonte,
                al_map_rgb(0, 0, 0),
                largura / 2,
                20,
                ALLEGRO_ALIGN_CENTER,
                "DuoFit - Regra dos 3 Passos"
            );

            // -------------------------
            // CONTADOR DE PASSOS
            // -------------------------

            char textoPassos[100];

            sprintf(
                textoPassos,
                "Passos com a bola: %d / 3",
                passosComBola
            );

            al_draw_text(
                fonte,
                al_map_rgb(0, 0, 0),
                largura / 2,
                60,
                ALLEGRO_ALIGN_CENTER,
                textoPassos
            );

            // -------------------------
            // ESCOLHER SPRITE
            // -------------------------

            ALLEGRO_BITMAP *spriteAtual;

            if (pulando) {

                spriteAtual = anaPulando;

            } else if (andando) {

                if (ultimaDirecao == -1) {

                    spriteAtual =
                        anaAndandoEsquerda1;

                } else {

                    if (frameAndando == 0) {

                        spriteAtual =
                            anaAndandoDireita1;

                    } else {

                        spriteAtual =
                            anaAndandoDireita2;
                    }
                }

            } else {

                if (ultimaDirecao == -1) {

                    spriteAtual =
                        anaParadaEsquerda;

                } else {

                    spriteAtual =
                        anaParadaDireita;
                }
            }

            // -------------------------
            // ESPELHAR PULO
            // -------------------------

            int flags = 0;

            if (
                pulando &&
                ultimaDirecao == -1
            ) {
                flags = ALLEGRO_FLIP_HORIZONTAL;
            }

            // -------------------------
            // DESENHAR ANA
            // -------------------------

            al_draw_scaled_bitmap(
                spriteAtual,

                0,
                0,

                al_get_bitmap_width(spriteAtual),
                al_get_bitmap_height(spriteAtual),

                anaX,
                anaY,

                larguraAna,
                alturaAna,

                flags
            );

            // -------------------------
            // DESENHAR BOLA
            // -------------------------

            if (estaComBola) {

                float bolaX;
                float bolaY;

                float tamanhoBola = 42;

                // -------------------------
                // PULANDO
                // -------------------------

                if (pulando) {

                    if (ultimaDirecao == 1) {

                        // mão direita durante o pulo
                        bolaX = anaX + 105;
                        bolaY = anaY + 65;

                    } else {

                        // mão esquerda durante o pulo
                        bolaX = anaX - 5;
                        bolaY = anaY + 65;
                    }

                }

                // -------------------------
                // ANDANDO
                // -------------------------

                else if (andando) {

                    if (ultimaDirecao == 1) {

                        bolaX = anaX + 105;
                        bolaY = anaY + 95;

                    } else {

                        bolaX = anaX - 5;
                        bolaY = anaY + 95;
                    }

                }

                // -------------------------
                // PARADA
                // -------------------------

                else {

                    if (ultimaDirecao == 1) {

                        bolaX = anaX + 100;
                        bolaY = anaY + 100;

                    } else {

                        bolaX = anaX;
                        bolaY = anaY + 100;
                    }
                }

                al_draw_scaled_bitmap(
                    bola,

                    0,
                    0,

                    al_get_bitmap_width(bola),
                    al_get_bitmap_height(bola),

                    bolaX,
                    bolaY,

                    tamanhoBola,
                    tamanhoBola,

                    0
                );
            }

            // -------------------------
            // CONTROLES
            // -------------------------

            al_draw_text(
                fonte,
                al_map_rgb(0, 0, 0),
                largura / 2,
                altura - 60,
                ALLEGRO_ALIGN_CENTER,
                "A = esquerda | D = direita | W = pular"
            );

            // -------------------------
            // POPUP INFRACAO
            // -------------------------

            if (mostrarInfracao) {

                al_draw_filled_rectangle(
                    largura / 2 - 300,
                    altura / 2 - 140,
                    largura / 2 + 300
                    altura / 2 + 140,
                    al_map_rgb(70, 20, 20)
                );

                al_draw_rectangle(
                    largura / 2 - 300,
                    altura / 2 - 140,
                    largura / 2 + 300,
                    altura / 2 + 140,
                    al_map_rgb(255, 255, 255),
                    3
                );

                al_draw_text(
                    fonte,
                    al_map_rgb(255, 80, 80),
                    largura / 2,
                    altura / 2 - 90,
                    ALLEGRO_ALIGN_CENTER,
                    "INFRACAO!"
                );

                al_draw_text(
                    fonte,
                    al_map_rgb(255, 255, 255),
                    largura / 2,
                    altura / 2 - 35,
                    ALLEGRO_ALIGN_CENTER,
                    "Voce tentou dar mais de 3 passos"
                );

                al_draw_text(
                    fonte,
                    al_map_rgb(255, 255, 255),
                    largura / 2,
                    altura / 2,
                    ALLEGRO_ALIGN_CENTER,
                    "segurando a bola."
                );

                al_draw_text(
                    fonte,
                    al_map_rgb(255, 220, 0),
                    largura / 2,
                    altura / 2 + 45,
                    ALLEGRO_ALIGN_CENTER,
                    "No handebol isso nao e permitido."
                );

                al_draw_text(
                    fonte,
                    al_map_rgb(0, 255, 120),
                    largura / 2,
                    altura / 2 + 95,
                    ALLEGRO_ALIGN_CENTER,
                    "Pressione ENTER para continuar"
                );
            }

            al_flip_display();
        }
    }

    // -------------------------
    // LIBERAR MEMORIA
    // -------------------------

    al_destroy_bitmap(anaParadaDireita);
    al_destroy_bitmap(anaParadaEsquerda);
    al_destroy_bitmap(anaAndandoDireita1);
    al_destroy_bitmap(anaAndandoDireita2);
    al_destroy_bitmap(anaAndandoEsquerda1);
    al_destroy_bitmap(anaPulando);

    al_destroy_bitmap(bola);

    al_destroy_font(fonte);

    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(display);

    return 0;
}