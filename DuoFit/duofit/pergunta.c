// DuoFit - pergunta do final de cada fase e tela de pontos
// Devolve PROXIMA_FASE (ENTER), VOLTAR_MENU (ESC) ou FECHAR_JOGO (X da janela).

#include "duofit.h"

int fazer_pergunta(char* esporte, char* pergunta, char* opcoes[3], int correta,
    char* explicacao, int pontosFase, bool ultimaFase) {

    ALLEGRO_EVENT_QUEUE* fila = criar_fila(NULL);   // sem timer: so espera tecla

    // mesmas cores das telas do handebol
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR amarelo = al_map_rgb(255, 220, 0);
    ALLEGRO_COLOR laranja = al_map_rgb(255, 140, 0);
    ALLEGRO_COLOR verde = al_map_rgb(0, 255, 120);
    ALLEGRO_COLOR azul = al_map_rgb(20, 45, 100);

    char letras[3] = { 'A', 'B', 'C' };
    int tela = 0;             // 0 = pergunta, 1 = mostra a resposta, 2 = pontos
    int resposta = -1;        // -1 = nao respondeu, 0 = A, 1 = B, 2 = C
    int pontosPergunta = 0;   // 10 se acertar
    int retorno = -1;         // -1 = continua nessa tela
    char texto[200];

    while (retorno == -1) {

        // ---------- desenhar ----------
        if (tela == 0 || tela == 1) {
            al_clear_to_color(azul);
            sprintf(texto, "PERGUNTA - %s", esporte);
            escrever(fonteGrande, laranja, 40, texto);
            escrever(fonte, branco, 150, pergunta);

            // as 3 alternativas
            for (int i = 0; i < 3; i++) {
                ALLEGRO_COLOR corBotao = al_map_rgb(40, 90, 170);
                if (tela == 1 && i == correta) corBotao = al_map_rgb(20, 140, 70);         // certa fica verde
                else if (tela == 1 && i == resposta) corBotao = al_map_rgb(170, 40, 40);   // errada fica vermelha
                al_draw_filled_rounded_rectangle(200, 210 + i * 80, 1000, 270 + i * 80, 12, 12, corBotao);
                sprintf(texto, "%c) %s", letras[i], opcoes[i]);
                escrever(fonte, branco, 225 + i * 80, texto);
            }

            if (tela == 0) escrever(fonte, verde, 500, "Aperte A, B ou C");
            else {
                if (resposta == correta) escrever(fonte, verde, 480, "CERTO! +10 pontos");
                else escrever(fonte, al_map_rgb(255, 80, 80), 480, "Nao foi dessa vez.");
                escrever(fonte, amarelo, 525, explicacao);
                escrever(fonte, verde, 600, "ENTER para ver os pontos");
            }
            escrever(fonte, branco, 690, "ESC/BACKSPACE para o menu");
        }
        else {
            // na ultima fase, ganha o DuoFit so quem fez pelo menos 80% dos pontos
            bool ganhou = (pontosTotal >= PONTOS_PARA_GANHAR);

            // tela de pontos (verde igual a de vitoria, vermelha se perdeu o jogo)
            if (ultimaFase && !ganhou) al_clear_to_color(al_map_rgb(120, 25, 25));
            else al_clear_to_color(al_map_rgb(20, 100, 60));
            escrever(fonteGrande, branco, 60, "PONTOS FINAIS");
            escrever(fonte, amarelo, 150, esporte);
            sprintf(texto, "Pontos da fase: %d", pontosFase);
            escrever(fonte, branco, 220, texto);
            sprintf(texto, "Pergunta: +%d", pontosPergunta);
            escrever(fonte, branco, 265, texto);
            sprintf(texto, "Total da fase: %d", pontosFase + pontosPergunta);
            escrever(fonte, amarelo, 310, texto);
            sprintf(texto, "Total do DuoFit: %d", pontosTotal);
            escrever(fonteGrande, amarelo, 390, texto);

            if (ultimaFase) {
                sprintf(texto, "Para ganhar: %d de %d pontos (80%%)", PONTOS_PARA_GANHAR, PONTOS_MAXIMOS);
                escrever(fonte, branco, 470, texto);
                if (ganhou) escrever(fonteGrande, verde, 520, "VOCE GANHOU O DUOFIT!");
                else {
                    escrever(fonteGrande, al_map_rgb(255, 120, 120), 520, "VOCE PERDEU!");
                    sprintf(texto, "Faltaram %d pontos. Tente de novo pelo menu!", PONTOS_PARA_GANHAR - pontosTotal);
                    escrever(fonte, branco, 600, texto);
                }
                escrever(fonte, branco, 690, "ENTER ou ESC para voltar ao menu");
            }
            else escrever(fonte, branco, 690, "ENTER: proxima fase  |  ESC/BACKSPACE: menu");
        }
        al_flip_display();

        // ---------- esperar o jogador ----------
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) retorno = FECHAR_JOGO;
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;

            if (tecla == ALLEGRO_KEY_ESCAPE || tecla == ALLEGRO_KEY_BACKSPACE) retorno = VOLTAR_MENU;
            else if (tela == 0) {
                if (tecla == ALLEGRO_KEY_A) resposta = 0;
                if (tecla == ALLEGRO_KEY_B) resposta = 1;
                if (tecla == ALLEGRO_KEY_C) resposta = 2;

                if (resposta != -1) {
                    if (resposta == correta) pontosPergunta = 10;
                    pontosTotal = pontosTotal + pontosFase + pontosPergunta;
                    tela = 1;
                }
            }
            else if (tecla == ALLEGRO_KEY_ENTER) {
                if (tela == 1) tela = 2;
                else retorno = PROXIMA_FASE;
            }
        }
    }

    al_destroy_event_queue(fila);
    return retorno;
}
