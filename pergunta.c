// DuoFit - pergunta do final de cada fase e tela de pontos
// Devolve PROXIMA_FASE (ENTER), VOLTAR_MENU (ESC) ou FECHAR_JOGO (X da janela).

#include "duofit.h"

int fazer_pergunta(char* esporte, char* pergunta, char* opcoes[3], int correta,
    char* explicacao, int pontosFase, bool ultimaFase) {

    ALLEGRO_EVENT_QUEUE* fila = criar_fila(NULL);   // sem timer: so espera tecla

    char letras[3] = { 'A', 'B', 'C' };
    int tela = 0;             // 0 = pergunta, 1 = mostra a resposta, 2 = pontos
    int resposta = -1;        // -1 = nao respondeu, 0 = A, 1 = B, 2 = C
    int pontosPergunta = 0;   // 10 se acertar
    int retorno = -1;         // -1 = continua nessa tela
    char texto[200];
    bool novoRecorde = false;
    int marcada = -1;         // alternativa que esta embaixo do mouse

    // qual fase e essa: 0 handebol, 1 atletismo, 2 basquete
    int numero = 0;
    if (strcmp(esporte, "ATLETISMO") == 0) numero = 1;
    if (strcmp(esporte, "BASQUETE") == 0) numero = 2;

    while (retorno == -1) {

        // ---------- desenhar (tudo na lousa) ----------
        al_clear_to_color(al_map_rgb(150, 100, 60));
        desenhar_lousa(28, 28, 1172, 690);

        if (tela == 0 || tela == 1) {
            escrever(fonteGizTitulo, GIZ_AMARELO, 45, "PERGUNTA");
            escrever(fonteGiz, GIZ_BRANCO, 130, esporte);
            al_draw_line(240, 178, 960, 174, GIZ_BRANCO, 2);
            escrever(fonteGiz, GIZ_BRANCO, 200, pergunta);

            // as 3 alternativas, com a letra dentro de um circulo
            for (int i = 0; i < 3; i++) {
                float y = 270 + i * 70;
                ALLEGRO_COLOR cor = GIZ_BRANCO;
                if (tela == 0 && i == marcada) cor = GIZ_AMARELO;          // mouse em cima
                if (tela == 1 && i == correta) cor = GIZ_VERDE;            // certa fica verde
                else if (tela == 1 && i == resposta) cor = GIZ_VERMELHO;   // errada fica vermelha

                al_draw_circle(420, y + 20, 22, cor, 3);
                sprintf(texto, "%c", letras[i]);
                al_draw_text(fonteGiz, cor, 420, y, ALLEGRO_ALIGN_CENTER, texto);
                al_draw_text(fonteGiz, cor, 465, y, ALLEGRO_ALIGN_LEFT, opcoes[i]);

                // na resposta: visto na certa e risco na errada
                if (tela == 1 && i == correta) {
                    al_draw_line(345, y + 20, 360, y + 36, GIZ_VERDE, 5);
                    al_draw_line(360, y + 36, 388, y, GIZ_VERDE, 5);
                }
                else if (tela == 1 && i == resposta) al_draw_line(390, y + 45, 455, y - 5, GIZ_VERMELHO, 5);
            }

            if (tela == 0) {
                escrever(fonteGiz, GIZ_AMARELO, 520, "Acertar vale +10 pontos");
                escrever(fonteGiz, GIZ_BRANCO, 625, "Aperte A, B ou C, ou clique   |   ESC: menu");
            }
            else {
                if (resposta == correta) escrever(fonteGiz, GIZ_VERDE, 495, "CERTO! +10 pontos");
                else escrever(fonteGiz, GIZ_VERMELHO, 495, "Nao foi dessa vez.");
                escrever(fonteGiz, GIZ_AMARELO, 550, explicacao);
                escrever(fonteGiz, GIZ_BRANCO, 625, "ENTER ou clique: ver os pontos   |   ESC: menu");
            }
        }
        else {
            // na ultima fase, ganha o DuoFit so quem fez pelo menos 80% dos pontos
            bool ganhou = (pontosTotal >= PONTOS_PARA_GANHAR);

            escrever(fonteGizTitulo, GIZ_AMARELO, 40, "PONTOS FINAIS");
            escrever(fonteGiz, GIZ_BRANCO, 125, esporte);

            // tabela: nome na esquerda, valor na direita e uma linha embaixo
            char* nomes[3] = { "Pontos da fase", "Pergunta", "Total da fase" };
            int valores[3] = { pontosFase, pontosPergunta, pontosFase + pontosPergunta };
            for (int i = 0; i < 3; i++) {
                float y = 180 + i * 55;
                ALLEGRO_COLOR cor = (i == 2) ? GIZ_AMARELO : GIZ_BRANCO;
                al_draw_text(fonteGiz, cor, 330, y, ALLEGRO_ALIGN_LEFT, nomes[i]);
                sprintf(texto, i == 1 ? "+%d" : "%d", valores[i]);
                al_draw_text(fonteGiz, cor, 870, y, ALLEGRO_ALIGN_RIGHT, texto);
                al_draw_line(325, y + 45, 875, y + 45, al_map_rgba(120, 125, 120, 128), 2);
            }

            // total do jogo circulado
            escrever(fonteGiz, GIZ_BRANCO, 360, "Total do DuoFit");
            sprintf(texto, "%d", pontosTotal);
            escrever(fonteGizTitulo, GIZ_AMARELO, 395, texto);
            al_draw_ellipse(600, 443, 130, 42, GIZ_BRANCO, 3);

            // recorde do lado do total
            sprintf(texto, "Recorde: %d", recorde);
            al_draw_text(fonteGiz, GIZ_BRANCO, 900, 420, ALLEGRO_ALIGN_LEFT, texto);
            if (novoRecorde) al_draw_text(fonteGiz, GIZ_VERDE, 900, 460, ALLEGRO_ALIGN_LEFT, "NOVO!");

            if (ultimaFase) {
                if (ganhou) escrever(fonteGizTitulo, GIZ_VERDE, 485, "VOCE GANHOU O DUOFIT!");
                else escrever(fonteGizTitulo, GIZ_VERMELHO, 485, "VOCE PERDEU!");
                if (ganhou) sprintf(texto, "Para ganhar: %d de %d pontos (80%%)", PONTOS_PARA_GANHAR, PONTOS_MAXIMOS);
                else sprintf(texto, "Faltaram %d pontos. Tente de novo pelo menu!", PONTOS_PARA_GANHAR - pontosTotal);
                escrever(fonteGiz, GIZ_BRANCO, 570, texto);
                escrever(fonteGiz, GIZ_BRANCO, 630, "ENTER, clique ou ESC para voltar ao menu");
            }
            else escrever(fonteGiz, GIZ_BRANCO, 625, "ENTER ou clique: proxima fase   |   ESC: menu");
        }

        al_flip_display();

        // ---------- esperar o jogador ----------
        ALLEGRO_EVENT evento;
        esperar_evento(fila, &evento);

        // qual alternativa esta embaixo do mouse (area da letra e do texto)
        if (evento.type == ALLEGRO_EVENT_MOUSE_AXES || clicou(evento)) {
            marcada = -1;
            for (int i = 0; i < 3; i++) {
                float y = 270 + i * 70;
                if (evento.mouse.x >= 390 && evento.mouse.x <= 900 &&
                    evento.mouse.y >= y - 10 && evento.mouse.y <= y + 50) marcada = i;
            }
        }
        // clique em uma alternativa responde; nas outras telas o clique vale como ENTER
        if (tela == 0 && clicou(evento)) {
            if (marcada != -1) resposta = marcada;
        }
        else clique_vira_enter(&evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) retorno = FECHAR_JOGO;
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;

            if (tecla == ALLEGRO_KEY_ESCAPE || tecla == ALLEGRO_KEY_BACKSPACE) retorno = VOLTAR_MENU;
            else if (tela == 0) {
                if (tecla == ALLEGRO_KEY_A) resposta = 0;
                if (tecla == ALLEGRO_KEY_B) resposta = 1;
                if (tecla == ALLEGRO_KEY_C) resposta = 2;

            }
            else if (tecla == ALLEGRO_KEY_ENTER) {
                if (tela == 1) tela = 2;
                else retorno = PROXIMA_FASE;
            }
        }

        // respondeu agora (pelo teclado ou pelo mouse): soma os pontos
        if (tela == 0 && resposta != -1) {
            if (resposta == correta) pontosPergunta = 10;
            // jogar a mesma fase de novo troca os pontos dela, em vez de somar outra vez
            pontosPorFase[numero] = pontosFase + pontosPergunta;
            pontosTotal = pontosPorFase[0] + pontosPorFase[1] + pontosPorFase[2];
            novoRecorde = atualizar_recorde(pontosTotal);
            tela = 1;
        }
    }

    al_destroy_event_queue(fila);
    return retorno;
}
