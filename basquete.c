#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "rico_sprites.h"
#include "fases.h"
#include "navegacao.h"

#define ALTURA_RICO   200.0f   /* altura do sprite na tela (px) */
#define CHAO_Y        650.0f   /* onde ficam os pes do Rico     */
#define VEL_JOGADOR   5.0f
#define GRAVIDADE     0.5f

static void desenhar_sprite(ALLEGRO_BITMAP* bmp, float centroX,
    float pesY, int flip) {
    if (!bmp) return;
    float escala = ALTURA_RICO / al_get_bitmap_height(bmp);
    float larg = al_get_bitmap_width(bmp) * escala;
    al_draw_scaled_bitmap(bmp,
        0, 0, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
        centroX - larg / 2, pesY - ALTURA_RICO, larg, ALTURA_RICO,
        flip ? ALLEGRO_FLIP_HORIZONTAL : 0);
}


/* Desenha o ginasio inteiro: parede, torcida, piso, linhas e cesta */
static void desenhar_quadra(ALLEGRO_FONT* fonte) {
    int i, r, c;

    /* Parede com degrade (faixas de 10 px) */
    for (i = 0; i < 63; i++) {
        float t = i / 62.0f;
        al_draw_filled_rectangle(0, i * 10, 1200, i * 10 + 10,
            al_map_rgb(47 + (int)(96 * t), 59 + (int)(102 * t),
                89 + (int)(107 * t)));
    }

    /* Letreiro */
    al_draw_filled_rectangle(380, 50, 820, 120, al_map_rgb(214, 40, 40));
    al_draw_text(fonte, al_map_rgb(255, 255, 255), 600, 68,
        ALLEGRO_ALIGN_CENTER, "DUOFIT ARENA");

    /* Arquibancada */
    for (i = 0; i < 4; i++)
        al_draw_filled_rectangle(0, 420 + i * 52, 1200, 472 + i * 52,
            (i % 2) ? al_map_rgb(91, 59, 42) : al_map_rgb(109, 71, 51));

    /* Torcida */
    ALLEGRO_COLOR cores[5] = {
        al_map_rgb(229, 50, 45), al_map_rgb(242, 193, 78),
        al_map_rgb(78, 168, 222), al_map_rgb(244, 244, 244),
        al_map_rgb(123, 211, 137)
    };
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 40; c++) {
            float px = 15 + c * 30 + (r % 2) * 15;
            float py = 462 + r * 52;
            al_draw_filled_circle(px, py, 10, cores[(c * 3 + r * 2) % 5]);
            al_draw_filled_circle(px, py - 15, 7, al_map_rgb(232, 184, 148));
        }
    }

    /* Faixa escura entre torcida e quadra */
    al_draw_filled_rectangle(0, 622, 1200, 632, al_map_rgb(34, 34, 34));

    /* Piso de madeira com degrade */
    for (i = 0; i < 12; i++) {
        float t = i / 11.0f;
        al_draw_filled_rectangle(0, 630 + i * 10, 1200, 640 + i * 10,
            al_map_rgb(207 - (int)(39 * t), 149 - (int)(37 * t),
                86 - (int)(28 * t)));
    }

    /* Tabuas em perspectiva */
    for (i = -10; i <= 30; i++)
        al_draw_line(i * 60, 630, 600 + (i * 60 - 600) * 1.6f, 750,
            al_map_rgb(175, 120, 70), 2);

    /* Garrafao */
    al_draw_filled_triangle(880, 634, 1130, 634, 1190, 748,
        al_map_rgb(160, 70, 50));
    al_draw_filled_triangle(880, 634, 1190, 748, 820, 748,
        al_map_rgb(160, 70, 50));
    al_draw_line(880, 634, 1130, 634, al_map_rgb(255, 255, 255), 4);
    al_draw_line(1130, 634, 1190, 748, al_map_rgb(255, 255, 255), 4);
    al_draw_line(880, 634, 820, 748, al_map_rgb(255, 255, 255), 4);

    /* Linha de tres pontos */
    al_draw_elliptical_arc(1080, 634, 430, 112,
        ALLEGRO_PI / 2, ALLEGRO_PI / 2, al_map_rgb(255, 255, 255), 4);

    /* Linha lateral */
    al_draw_filled_rectangle(0, 630, 1200, 634, al_map_rgb(255, 255, 255));

    /* Zona ideal de arremesso */
    al_draw_filled_rectangle(352, 652, 442, 660, al_map_rgb(255, 230, 0));

    /* Cesta: suporte, tabela, aro e rede */
    al_draw_line(1160, 230, 1160, 632, al_map_rgb(85, 85, 85), 12);
    al_draw_line(1086, 230, 1160, 230, al_map_rgb(85, 85, 85), 10);
    al_draw_filled_rectangle(1072, 140, 1088, 290, al_map_rgb(255, 255, 255));
    al_draw_rectangle(1072, 140, 1088, 290, al_map_rgb(0, 0, 0), 3);
    al_draw_filled_rectangle(1074, 232, 1086, 282, al_map_rgb(214, 40, 40));
    al_draw_line(980, 280, 1072, 280, al_map_rgb(255, 59, 29), 6);
    for (i = 0; i <= 4; i++)
        al_draw_line(985 + i * 24, 282, 995 + i * 20, 330,
            al_map_rgb(90, 70, 60), 2);
}

/* Programa independente: movimento, salto e arremesso na cesta. */
void jogar_basquete(void) {
    ALLEGRO_DISPLAY *janela = al_create_display(1200, 750);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_FONT *fonte = al_load_ttf_font(
        "/System/Library/Fonts/Supplemental/Arial.ttf", 24, 0);
    if (!fonte) fonte = al_create_builtin_font();
    if (!janela || !timer || !fila || !fonte) {
        fprintf(stderr, "Erro ao criar a janela ou os recursos do jogo.\n");
        if (fonte) al_destroy_font(fonte);
        if (fila) al_destroy_event_queue(fila);
        if (timer) al_destroy_timer(timer);
        if (janela) al_destroy_display(janela);
        return;
    }
    al_set_window_title(janela, "DuoFit - Basquete");
    ALLEGRO_BITMAP *atlasRico = carregar_atlas_rico();
    ALLEGRO_BITMAP *parado = recortar_rico(atlasRico, RICO_PARADO);
    ALLEGRO_BITMAP *andando = recortar_rico(atlasRico, RICO_ANDANDO);
    ALLEGRO_BITMAP *pulando = recortar_rico(atlasRico, RICO_PULANDO);
    if (!parado || !andando || !pulando) {
        fprintf(stderr, "Execute o programa na pasta do projeto, junto de assets/.\n");
        if (parado) al_destroy_bitmap(parado);
        if (andando) al_destroy_bitmap(andando);
        if (pulando) al_destroy_bitmap(pulando);
        if (atlasRico) al_destroy_bitmap(atlasRico);
        al_destroy_font(fonte);
        al_destroy_event_queue(fila);
        al_destroy_timer(timer);
        al_destroy_display(janela);
        return;
    }

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_mouse_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));
    bool sair = false, esquerda = false, direita = false;
    bool bolaNoAr = false, marcou = false;
    int pontos = 0, tentativas = 0, flip = 0;
    float x = 397, pes = CHAO_Y, salto = 0;
    float bolaX = x + 45, bolaY = pes - 95, bolaVX = 0, bolaVY = 0;
    al_start_timer(timer);
    while (!sair) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);
        if (pediu_voltar(&evento)) break;
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) sair = true;
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;
            if (tecla == ALLEGRO_KEY_ESCAPE) sair = true;
            if (tecla == ALLEGRO_KEY_A || tecla == ALLEGRO_KEY_LEFT) esquerda = true;
            if (tecla == ALLEGRO_KEY_D || tecla == ALLEGRO_KEY_RIGHT) direita = true;
            if (tecla == ALLEGRO_KEY_SPACE && pes >= CHAO_Y) salto = -12;
            if (tecla == ALLEGRO_KEY_E && !bolaNoAr) {
                bolaNoAr = true;
                marcou = false;
                tentativas++;
                bolaX = x + 45;
                bolaY = pes - 95;
                /* Trajetoria mira o aro; a faixa amarela rende tres pontos. */
                float duracao = 65;
                bolaVX = (1025 - bolaX) / duracao;
                bolaVY = (280 - bolaY - 0.5f * GRAVIDADE *
                    duracao * duracao) / duracao;
            }
            if (tecla == ALLEGRO_KEY_R) {
                x = 397;
                pes = CHAO_Y;
                salto = 0;
                bolaNoAr = false;
                pontos = tentativas = 0;
            }
        }
        if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            int tecla = evento.keyboard.keycode;
            if (tecla == ALLEGRO_KEY_A || tecla == ALLEGRO_KEY_LEFT) esquerda = false;
            if (tecla == ALLEGRO_KEY_D || tecla == ALLEGRO_KEY_RIGHT) direita = false;
        }
        if (evento.type == ALLEGRO_EVENT_TIMER) {
            if (esquerda != direita) {
                x += direita ? VEL_JOGADOR : -VEL_JOGADOR;
                flip = esquerda;
            }
            if (x < 60) x = 60;
            if (x > 900) x = 900;
            if (pes < CHAO_Y || salto < 0) {
                pes += salto;
                salto += GRAVIDADE;
                if (pes >= CHAO_Y) { pes = CHAO_Y; salto = 0; }
            }
            if (bolaNoAr) {
                float anteriorY = bolaY;
                bolaX += bolaVX;
                bolaY += bolaVY + 0.5f * GRAVIDADE;
                bolaVY += GRAVIDADE;
                if (!marcou && bolaVY > 0 && anteriorY < 280 &&
                    bolaY >= 280 && bolaX >= 985 && bolaX <= 1067) {
                    /* A origem do arremesso e preservada pela velocidade. */
                    float origemX = 1025 - bolaVX * 65 - 45;
                    pontos += (origemX >= 352 && origemX <= 442) ? 3 : 2;
                    marcou = true;
                }
                if (bolaY > 730 || bolaX > 1220 || bolaX < -20)
                    bolaNoAr = false;
            } else {
                bolaX = x + (flip ? -45 : 45);
                bolaY = pes - 95;
            }
            desenhar_quadra(fonte);
            ALLEGRO_BITMAP *sprite = pes < CHAO_Y ? pulando :
                (esquerda != direita ? andando : parado);
            desenhar_sprite(sprite, x, pes, flip);
            al_draw_filled_circle(bolaX, bolaY, 15, al_map_rgb(235, 120, 30));
            al_draw_circle(bolaX, bolaY, 15, al_map_rgb(60, 35, 20), 2);
            al_draw_line(bolaX - 15, bolaY, bolaX + 15, bolaY,
                al_map_rgb(60, 35, 20), 2);
            al_draw_textf(fonte, al_map_rgb(255, 255, 255), 24, 20, 0,
                "Pontos: %d | Arremessos: %d", pontos, tentativas);
            al_draw_text(fonte, al_map_rgb(255, 255, 255), 600, 685,
                ALLEGRO_ALIGN_CENTER, "A/D ou setas: mover | Espaco: saltar | E: arremessar");
            al_draw_text(fonte, al_map_rgb(255, 255, 255), 600, 718,
                ALLEGRO_ALIGN_CENTER, "Faixa amarela: 3 pontos | R: reiniciar | Esc: menu");
            desenhar_voltar(fonte);
            al_flip_display();
        }
    }
    al_destroy_bitmap(parado);
    al_destroy_bitmap(andando);
    al_destroy_bitmap(pulando);
    al_destroy_bitmap(atlasRico);
    al_destroy_font(fonte);
    al_destroy_event_queue(fila);
    al_destroy_timer(timer);
    al_destroy_display(janela);
    return;
}
