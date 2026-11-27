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

        // ---------- desenhar (tudo na lousa) ----------
        al_clear_to_color(al_map_rgb(150, 100, 60));
        desenhar_lousa(28, 28, 1172, 690);

        escrever(fonteGiz, GIZ_BRANCO, 42, "ESCOLA VITORIA");
        escrever(fonteGizTitulo, GIZ_AMARELO, 82, "DuoFit");
        al_draw_line(485, 168, 715, 162, GIZ_AMARELO, 4);

        char textoRecorde[50];
        sprintf(textoRecorde, "Recorde: %d pontos", recorde);
        escrever(fonteGiz, GIZ_BRANCO, 192, textoRecorde);

        // fotos dos jogadores presas na lousa com fita (Ana na esquerda, Rico na direita)
        char* nomes[2] = { "ANA", "RICO" };
        ALLEGRO_BITMAP* fotos[2] = { fotoAna, fotoRico };
        for (int j = 0; j < 2; j++) {
            float x = 95 + j * 760;
            al_draw_filled_rectangle(x + 8, 218, x + 258, 598, al_map_rgba(0, 0, 0, 70));            // sombra
            al_draw_filled_rectangle(x, 210, x + 250, 590, al_map_rgb(245, 245, 240));               // moldura
            al_draw_filled_rectangle(x + 15, 225, x + 235, 535, al_map_rgb(200, 220, 240));          // fundo da foto
            desenhar(fotos[j], x + 25, 230, 200, 300, 0);
            al_draw_text(fonteGiz, al_map_rgb(20, 45, 100), x + 125, 543, ALLEGRO_ALIGN_CENTER, nomes[j]);
            // fita: dois triangulos formando um pedaco meio torto
            al_draw_filled_triangle(x + 85, 196, x + 168, 200, x + 165, 226, al_map_rgba(190, 175, 110, 190));
            al_draw_filled_triangle(x + 85, 196, x + 165, 226, x + 82, 222, al_map_rgba(190, 175, 110, 190));
        }

        for (int i = 0; i < 4; i++) {
            // a fase do botao i e i + 1 (o botao 3 e o SAIR, sempre liberado)
            bool bloqueado = (i < 3 && i + 1 > faseLiberada);
            float y = 262 + i * 80;   // cada opcao fica 80 pixels abaixo da outra

            ALLEGRO_COLOR cor = GIZ_BRANCO;
            if (bloqueado) cor = al_map_rgb(130, 155, 140);   // apagada
            else if (opcao == i) cor = GIZ_AMARELO;
            escrever(fonteGiz, cor, y + 10, botoes[i]);

            // a opcao marcada fica circulada, com uma seta do lado
            if (!bloqueado && opcao == i) {
                al_draw_ellipse(600, y + 30, 165, 34, GIZ_AMARELO, 3);
                al_draw_filled_triangle(385, y + 15, 385, y + 45, 412, y + 30, GIZ_AMARELO);
            }
            // cadeado desenhado a giz
            if (bloqueado) {
                al_draw_arc(755, y + 27, 9, ALLEGRO_PI, ALLEGRO_PI, cor, 3);
                al_draw_filled_rectangle(742, y + 27, 768, y + 47, cor);
            }
        }

        // dica de como liberar a proxima fase
        if (faseLiberada == 1) escrever(fonteGizPequena, GIZ_AMARELO, 578, "Passe do handebol para liberar o atletismo.");
        if (faseLiberada == 2) escrever(fonteGizPequena, GIZ_AMARELO, 578, "Passe do atletismo para liberar o basquete.");

        escrever(fonteGizPequena, GIZ_BRANCO, 635, "W/S ou mouse: escolher   |   ENTER ou clique: entrar");
        al_flip_display();

        // ---------- esperar o jogador ----------
        ALLEGRO_EVENT evento;
        esperar_evento(fila, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) escolha = 0;

        // mouse em cima de um botao liberado marca ele; clique entra
        if (evento.type == ALLEGRO_EVENT_MOUSE_AXES || clicou(evento)) {
            for (int i = 0; i < 4; i++) {
                bool liberado = (i == 3 || i + 1 <= faseLiberada);
                if (liberado && evento.mouse.x >= 400 && evento.mouse.x <= 800 &&
                    evento.mouse.y >= 262 + i * 80 && evento.mouse.y <= 322 + i * 80) {
                    opcao = i;
                    if (clicou(evento)) escolha = (i == 3) ? 0 : i + 1;
                }
            }
        }

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
