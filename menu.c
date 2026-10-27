// DuoFit - Menu principal. Cada esporte tem seu proprio arquivo .c.
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include "menu.h"

// Retorna 0 para sair ou o numero da modalidade (1 a 3).
int mostrar_menu(void) {
    ALLEGRO_DISPLAY *janela = al_create_display(1200, 750);
    ALLEGRO_EVENT_QUEUE *fila = al_create_event_queue();
    ALLEGRO_FONT *fonte = al_load_ttf_font("/System/Library/Fonts/Supplemental/Arial.ttf", 32, 0);
    if (!janela || !fila || !fonte) {
        printf("Erro ao abrir o menu.\n");
        if (fonte) al_destroy_font(fonte);
        if (fila) al_destroy_event_queue(fila);
        if (janela) al_destroy_display(janela);
        return 0;
    }
    al_set_window_title(janela, "DuoFit");
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_mouse_event_source());
    int opcao = 0;
    int escolha = -1;
    const char *botoes[4] = {"HANDEBOL", "ATLETISMO", "BASQUETE", "SAIR"};
    while (escolha == -1) {
        al_clear_to_color(al_map_rgb(20, 45, 100));
        al_draw_text(fonte, al_map_rgb(255, 255, 255), 600, 130, ALLEGRO_ALIGN_CENTER, "DuoFit");
        for (int i = 0; i < 4; i++) {
            ALLEGRO_COLOR cor = al_map_rgb(40, 90, 170);
            if (opcao == i) cor = al_map_rgb(255, 140, 0);
            al_draw_filled_rounded_rectangle(400, 280 + i * 90, 800, 345 + i * 90, 12, 12, cor);
            al_draw_text(fonte, al_map_rgb(255, 255, 255), 600, 290 + i * 90, ALLEGRO_ALIGN_CENTER, botoes[i]);
        }
        al_draw_text(fonte, al_map_rgb(255, 255, 255), 600, 700, ALLEGRO_ALIGN_CENTER,
            "Clique para escolher | W/S + ENTER | 1 handebol | 2 atletismo | 3 basquete");
        al_flip_display();
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) escolha = 0;
        if (evento.type == ALLEGRO_EVENT_MOUSE_AXES ||
            (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && evento.mouse.button == 1)) {
            for (int i = 0; i < 4; i++) {
                if (evento.mouse.x >= 400 && evento.mouse.x <= 800 &&
                    evento.mouse.y >= 280 + i * 90 && evento.mouse.y <= 345 + i * 90) {
                    opcao = i;
                    if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
                        escolha = (i == 3) ? 0 : i + 1;
                    break;
                }
            }
        }
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;
            if (tecla == ALLEGRO_KEY_ESCAPE) escolha = 0;
            else if (tecla == ALLEGRO_KEY_1) escolha = 1;
            else if (tecla == ALLEGRO_KEY_2) escolha = 2;
            else if (tecla == ALLEGRO_KEY_3) escolha = 3;
            else if (tecla == ALLEGRO_KEY_W || tecla == ALLEGRO_KEY_UP) {
                if (opcao > 0) opcao--;
            } else if (tecla == ALLEGRO_KEY_S || tecla == ALLEGRO_KEY_DOWN) {
                if (opcao < 3) opcao++;
            } else if (tecla == ALLEGRO_KEY_ENTER) {
                if (opcao == 3) escolha = 0;
                else escolha = opcao + 1;
            }
        }
    }
    al_destroy_font(fonte);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    return escolha;
}
