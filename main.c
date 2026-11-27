// DuoFit - Projeto Integrador II - Ciencia da Computacao - Senac
// Equipe: Nathali, Ana Helena e Gabriel
//
// main: liga o Allegro, abre a janela e troca entre o menu e as fases.

#include "duofit.h"
#include <stdlib.h>   // srand
#include <time.h>     // time

int main(int argc, char** argv) {

    if (!al_init()) {
        printf("Erro ao iniciar o Allegro.\n");
        return 1;
    }
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    srand(time(NULL));   // para o rand da goleira mudar a cada vez

    // o jogo e 2D, entao pedi para a janela nao guardar memoria de profundidade (usada em 3D)
    al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 0, ALLEGRO_SUGGEST);

    // uma janela so para o jogo todo (o menu e as fases desenham nela)
    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA, ALTURA);
    if (janela == NULL) {
        printf("Erro ao criar a janela.\n");
        return 1;
    }
    al_set_window_title(janela, "DuoFit");

    // No Visual Studio as vezes o programa roda em outra pasta.
    // Se nao achar a pasta assets aqui, sobe uma pasta e tenta de novo (ate 3 vezes).
    int subidas = 0;
    while (!al_filename_exists("assets/bola.png") && subidas < 3) { // mudar negaçao dentro do if

        al_change_directory("..");
        subidas++;
    }

    if (!carregar_recursos()) {
        printf("Erro: nao achei as imagens. Coloque a pasta assets junto do projeto.\n");
        liberar_recursos();
        al_destroy_display(janela);
        return 1;
    }

    carregar_recorde();   // depois do ajuste de pasta, para achar o arquivo

    // fase 0 = menu, 1 = handebol, 2 = atletismo, 3 = basquete
    int fase = 0;
    bool rodando = true;

    while (rodando) {
        if (fase == 0) {
            fase = mostrar_menu();
            if (fase == 1) {
                // comecou um jogo novo (sempre pelo handebol)
                pontosTotal = 0;
                for (int i = 0; i < 3; i++) pontosPorFase[i] = 0;
            }
            if (fase == 0) rodando = false;   // escolheu SAIR
        }
        else {
            int resultado;
            if (fase == 1) resultado = jogar_handebol();
            else if (fase == 2) resultado = jogar_atletismo();
            else resultado = jogar_basquete();

            // handebol -> atletismo -> basquete -> menu
            if (resultado == FECHAR_JOGO) rodando = false;
            else if (resultado == PROXIMA_FASE && fase < 3) fase++;
            else fase = 0;
        }
    }

    liberar_recursos();
    al_destroy_display(janela);
    return 0;
}
