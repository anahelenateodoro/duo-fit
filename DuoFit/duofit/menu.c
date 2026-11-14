// DuoFit - menu principal
// Devolve 0 para sair ou o numero da fase (1 handebol, 2 atletismo, 3 basquete).
// O jogo comeca pelo handebol: as outras fases ficam bloqueadas ate passar da anterior.

#include "duofit.h"

int mostrar_menu(void) {

    ALLEGRO_EVENT_QUEUE* fila = criar_fila(NULL);   // sem timer: so espera tecla

    char* botoes[4] = { "HANDEBOL", "ATLETISMO", "BASQUETE", "SAIR" };
    int opcao = 0;      // botao marcado (0 a 3), sempre comeca no handebol
    int escolha = -1;   // -1 = ainda nao escolheu

    while (escolha == -1) {

        // ---------- desenhar ----------
        al_clear_to_color(al_map_rgb(20, 45, 100));
        escrever(fonteMenu, al_map_rgb(255, 255, 255), 130, "DuoFit");

        // fotos dos jogadores decorando o menu, como porta-retratos:
        // moldura branca, a foto dentro e o nome embaixo (Ana na esquerda, Rico na direita)
        char* nomes[2] = { "ANA", "RICO" };
        ALLEGRO_BITMAP* fotos[2] = { fotoAna, fotoRico };
        for (int j = 0; j < 2; j++) {
            float x = 85 + j * 780;   // 85 = moldura da esquerda, 865 = moldura da direita
            al_draw_filled_rounded_rectangle(x, 195, x + 250, 600, 10, 10, al_map_rgb(245, 245, 240));
            al_draw_filled_rectangle(x + 15, 210, x + 235, 545, al_map_rgb(200, 220, 240));   // fundo da foto
            desenhar(fotos[j], x + 15, 212, 220, 330, 0);
            al_draw_text(fonteMenu, al_map_rgb(20, 45, 100), x + 125, 553, ALLEGRO_ALIGN_CENTER, nomes[j]);
        }

        for (int i = 0; i < 4; i++) {
            // a fase do botao i e i + 1 (o botao 3 e o SAIR, sempre liberado)
            bool bloqueado = (i < 3 && i + 1 > faseLiberada);

            // cada botao fica 90 pixels abaixo do outro
            if (bloqueado) {
                // fase bloqueada: sempre cinza, nunca fica marcada
                al_draw_filled_rounded_rectangle(400, 280 + i * 90, 800, 345 + i * 90, 12, 12, al_map_rgb(70, 75, 90));
                char texto[50];
                sprintf(texto, "%s (bloqueado)", botoes[i]);
                escrever(fonte, al_map_rgb(150, 150, 160), 298 + i * 90, texto);
            }
            else {
                ALLEGRO_COLOR cor = al_map_rgb(40, 90, 170);
                if (opcao == i) cor = al_map_rgb(255, 140, 0);   // botao marcado fica laranja
                al_draw_filled_rounded_rectangle(400, 280 + i * 90, 800, 345 + i * 90, 12, 12, cor);
                escrever(fonteMenu, al_map_rgb(255, 255, 255), 290 + i * 90, botoes[i]);
            }
        }

        // dica de como liberar a proxima fase
        if (faseLiberada == 1) escrever(fonte, al_map_rgb(255, 180, 60), 640, "Passe do handebol para liberar o atletismo.");
        if (faseLiberada == 2) escrever(fonte, al_map_rgb(255, 180, 60), 640, "Passe do atletismo para liberar o basquete.");
        escrever(fonteMenu, al_map_rgb(255, 255, 255), 700, "W/S para escolher | ENTER para entrar");
        al_flip_display();

        // ---------- esperar o jogador ----------
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) escolha = 0;

        if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;
            if (tecla == ALLEGRO_KEY_ESCAPE) escolha = 0;

            // W/S so andam pelos botoes liberados: 0 ate faseLiberada - 1, e o SAIR (3)
            if (tecla == ALLEGRO_KEY_W || tecla == ALLEGRO_KEY_UP) {
                if (opcao == 3) opcao = faseLiberada - 1;   // do SAIR pula para a ultima fase liberada
                else if (opcao > 0) opcao--;
            }
            if (tecla == ALLEGRO_KEY_S || tecla == ALLEGRO_KEY_DOWN) {
                if (opcao < faseLiberada - 1) opcao++;
                else opcao = 3;                             // pula as bloqueadas e vai para o SAIR
            }

            if (tecla == ALLEGRO_KEY_ENTER) {
                if (opcao == 3) escolha = 0;
                else escolha = opcao + 1;
            }
        }
    }

    al_destroy_event_queue(fila);
    return escolha;
}
