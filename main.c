// Entrada unica do DuoFit: inicializacao e navegacao entre modalidades.
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "fases.h"

int main(int argc, char **argv) {
    if (!al_init() || !al_install_keyboard() || !al_install_mouse() || !al_init_primitives_addon() || !al_init_image_addon()) {
        printf("Erro ao iniciar Allegro.\n");
        return 1;
    }
    al_init_font_addon();
    al_init_ttf_addon();
    if (argc > 1 && strcmp(argv[1], "--basquete") == 0) {
        jogar_basquete();
    }
    bool rodando = true;
    while (rodando) {
        int escolha = mostrar_menu();
        if (escolha == 1) jogar_handebol();
        else if (escolha == 2) jogar_atletismo();
        else if (escolha == 3) jogar_basquete();
        else rodando = false;
    }
    return 0;
}
