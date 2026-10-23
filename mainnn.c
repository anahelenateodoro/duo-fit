#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <math.h>

/* Pasta onde estao os PNGs (relativa a pasta de execucao).
   Ex.: "assets/" se voce colocar os PNGs em uma pasta assets. */
#define PASTA_SPRITES ""

#define ALTURA_RICO   200.0f   /* altura do sprite na tela (px) */
#define CHAO_Y        650.0f   /* onde ficam os pes do Rico     */
#define VEL_JOGADOR   5.0f
#define GRAVIDADE     0.5f

static ALLEGRO_BITMAP* carregar_sprite(const char* nome) {
    char caminho[256];
    snprintf(caminho, sizeof(caminho), "%s%s", PASTA_SPRITES, nome);
    ALLEGRO_BITMAP* bmp = al_load_bitmap(caminho);
    if (!bmp)
        printf("Aviso: nao foi possivel carregar %s\n", caminho);
    return bmp;
}

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

void jogar_basket(void) {}

int main() {
    /* Seguro chamar varias vezes, caso o main ja tenha inicializado */
    al_init();
    al_init_image_addon();
    al_init_primitiv