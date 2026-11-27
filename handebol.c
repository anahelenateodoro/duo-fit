

#include "duofit.h"
#include <stdlib.h>    // rand

// estados da fase: cada numero e uma tela diferente
#define REGRAS    1
#define HANDEBOL  2
#define VITORIA   3
#define DERROTA   4    // acabaram as tentativas: refaz a fase

// posicoes da quadra (CHAO, RICO_L, RICO_A e BOLA_T ficam no duofit.h)
#define AREA       780    // x da linha dos 6 metros
#define GOL_X      1110   // x da trave do gol
#define GOL_Y      330    // y do travessao (parte de cima do gol)
#define GOLEIRO_L  130
#define GOLEIRO_A  200

// Verifica se dois retangulos estao se encostando (colisao).
// Eles se encostam quando se sobrepoem no eixo X E no eixo Y ao mesmo tempo.
bool colidiu(float x1, float y1, float l1, float a1, float x2, float y2, float l2, float a2) {
    bool sobrepoeX = (x1 < x2 + l2) && (x1 + l1 > x2);
    bool sobrepoeY = (y1 < y2 + a2) && (y1 + a1 > y2);
    return sobrepoeX && sobrepoeY;
}

int jogar_handebol(void) {

    int retorno = VOLTAR_MENU;   // o que o main abre depois dessa fase
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);   // dispara 60 vezes por segundo (60 FPS)
    ALLEGRO_EVENT_QUEUE* fila = criar_fila(timer);       // fila onde chegam teclas, timer etc.

    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);

    // ---------- variaveis do jogo ----------
    int estado = REGRAS;     // qual tela esta aparecendo

    // Rico: posicao (x, y) e velocidade vertical
    float ricoX = 150, ricoY = CHAO - RICO_A, ricoVelY = 0;
    int direcao = 1;             // 1 = olhando para a direita, -1 = para a esquerda
    int tempoAndando = 0;        // enquanto for > 0, mostra a imagem "andando"
    bool noChao = true;
    bool jaMexeuNoAr = false;    // no ar ele so pode mudar de lugar 1 vez

    // bola
    bool comBola = true;         // true = bola na mao, false = bola voando
    int passos = 0;              // passos dados segurando a bola
    int tempoDrible = 0;         // enquanto for > 0, a bola esta quicando
    float bolaX = 0, bolaY = 0, bolaVelX = 0, bolaVelY = 0;
    float forca = 0;             // barra de forca, vai de 0 a 100
    float variacaoForca = 2;     // +2 enchendo, -2 esvaziando

    // goleira (Ana)
    float goleiroX = 960, goleiroY = CHAO - GOLEIRO_A;
    float goleiroVelX = -2;      // anda 2 pixels por frame
    float goleiroVelY = 0;       // diferente de 0 = esta pulando
    bool goleiroDecidiu = false; // ja sorteou se vai pular nesse arremesso?

    // placar e mensagens
    int gols = 0, arremessos = 0;
    int infracao = 0;            // 0 = nenhuma, 1 = mais de 3 passos, 2 = pisou na area
    char* resultado = NULL;      // texto "GOOOL!" etc. NULL = sem mensagem
    int tempoResultado = 0;      // quantos frames a mensagem ainda fica na tela
    bool reiniciar = true;       // true = volta tudo para a posicao inicial

    bool rodando = true, redesenhar = false;
    char texto[100], texto2[100];   // para montar textos com sprintf

    al_start_timer(timer);

    // LOOP PRINCIPAL: a cada volta espera um evento, trata ele e desenha a tela
    while (rodando) {

        ALLEGRO_EVENT evento;
        al_wait_for_event(fila, &evento);
        clique_vira_enter(&evento);

        // ESC/BACKSPACE volta para o menu e o X da janela fecha o jogo
        if (verificar_saida(evento) != -1) {
            retorno = verificar_saida(evento);
            rodando = false;
        }

        // =============================================
        // 1) TECLADO
        // =============================================
        if (rodando && evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            int tecla = evento.keyboard.keycode;

            // regras ou derrota: ENTER comeca a fase do zero
            if ((estado == REGRAS || estado == DERROTA) && tecla == ALLEGRO_KEY_ENTER) {
                gols = 0; arremessos = 0; infracao = 0; resultado = NULL;
                reiniciar = true;
                estado = HANDEBOL;
            }
            // vitoria: libera o atletismo no menu e abre a pergunta do final
            else if (estado == VITORIA && tecla == ALLEGRO_KEY_ENTER) {
                if (faseLiberada < 2) faseLiberada = 2;
                char* opcoes[3] = { "2 passos", "3 passos", "5 passos" };
                retorno = fazer_pergunta("HANDEBOL",
                    "Quantos passos pode dar quem esta segurando a bola?", opcoes, 1,
                    "No maximo 3 passos. Para andar mais, e preciso quicar a bola.",
                    calcular_pontos(arremessos - gols, 2), false);
                rodando = false;
            }
            // com o aviso de infracao aberto, so o ENTER funciona
            else if (estado == HANDEBOL && infracao != 0) {
                if (tecla == ALLEGRO_KEY_ENTER) {
                    if (infracao == 2) reiniciar = true;   // pisou na area: volta para o comeco
                    infracao = 0;
                }
            }
            // so aceita comandos se nao tiver mensagem de gol/defesa na tela
            else if (estado == HANDEBOL && resultado == NULL) {

                // A/D andar, W pular, S quicar (a funcao esta no comum.c)
                if (mover_rico(tecla, 3, comBola, &ricoX, &ricoVelY, &direcao, &noChao,
                    &jaMexeuNoAr, &tempoAndando, &passos, &tempoDrible)) {
                    infracao = 1;   // regra dos 3 passos
                }

                // ESPACO = arremessar: a bola sai da mao do Rico
                if (tecla == ALLEGRO_KEY_SPACE && comBola && tempoDrible == 0) {
                    bolaX = ricoX + (direcao == 1 ? 105 : -5);
                    bolaY = ricoY + (noChao ? 100 : 20);   // no pulo, a bola fica no punho levantado
                    bolaVelX = 18 * direcao;               // velocidade para o lado
                    bolaVelY = -1 - forca * 0.12;          // mais forca = sobe mais
                    comBola = false;
                    goleiroDecidiu = false;
                    arremessos++;
                }
            }
        }

        // =============================================
        // 2) ATUALIZAR O JOGO (a cada tique do timer)
        // =============================================
        if (evento.type == ALLEGRO_EVENT_TIMER) redesenhar = true;

        if (evento.type == ALLEGRO_EVENT_TIMER && estado == HANDEBOL) {

            // ----- volta tudo para o inicio -----
            if (reiniciar) {
                ricoX = 150; ricoY = CHAO - RICO_A; ricoVelY = 0; noChao = true; direcao = 1;
                comBola = true; passos = 0; tempoDrible = 0;
                goleiroX = 960; goleiroY = CHAO - GOLEIRO_A; goleiroVelY = 0;
                reiniciar = false;
            }

            // ----- pulo do Rico (a fisica esta explicada no comum.c) -----
            atualizar_pulo(&ricoY, &ricoVelY, &noChao);

            // nao deixa sair da tela
            if (ricoX < 0) ricoX = 0;
            if (ricoX > LARGURA - RICO_L) ricoX = LARGURA - RICO_L;

            // contadores diminuem 1 por frame ate chegar em 0
            if (tempoAndando > 0) tempoAndando--;
            if (tempoDrible > 0) tempoDrible--;

            // ----- regra da area -----
            // o meio do corpo do Rico (ricoX + metade da largura) passou da linha
            if (comBola && noChao && infracao == 0 && ricoX + RICO_L / 2 > AREA) infracao = 2;

            // ----- barra de forca: vai de 0 ate 100 e volta -----
            if (comBola) {
                forca = forca + variacaoForca;
                if (forca >= 100 || forca <= 0) variacaoForca = -variacaoForca;   // inverte o sentido
            }

            // ----- goleira anda de um lado para o outro -----
            goleiroX = goleiroX + goleiroVelX;
            if (goleiroX < 900 || goleiroX > 975) goleiroVelX = -goleiroVelX;

            // ----- bola voando -----
            if (!comBola && resultado == NULL) {

                // mesma fisica do pulo, so que com gravidade menor
                bolaX = bolaX + bolaVelX;
                bolaY = bolaY + bolaVelY;
                bolaVelY = bolaVelY + 0.4;

                // quica no chao perdendo 40% da velocidade
                if (bolaY + BOLA_T >= CHAO) {
                    bolaY = CHAO - BOLA_T;
                    bolaVelY = -bolaVelY * 0.6;
                }

                // quando a bola chega perto, a goleira sorteia se pula
                // rand() % 2 da 0 ou 1 -> 50% de chance
                if (!goleiroDecidiu && bolaX > goleiroX - 280) {
                    goleiroDecidiu = true;
                    if (rand() % 2 == 1) goleiroVelY = -15;
                }

                // bateu na goleira? (uso uma caixa um pouco menor que a imagem)
                if (colidiu(bolaX, bolaY, BOLA_T, BOLA_T, goleiroX + 25, goleiroY + 20, GOLEIRO_L - 50, GOLEIRO_A - 20)) {
                    resultado = "DEFESA DA GOLEIRA!";
                }
                // chegou na linha do gol: passou por baixo do travessao e gol
                else if (bolaX + BOLA_T >= GOL_X) {
                    if (bolaY >= GOL_Y) {
                        resultado = "GOOOL!";
                        gols++;
                        tocar_som_gol();
                    }
                    else resultado = "PARA FORA!";
                }
                // saiu pela esquerda da tela
                else if (bolaX < -BOLA_T) resultado = "PARA FORA!";

                if (resultado != NULL) tempoResultado = 90;   // 90 frames = 1,5 segundo
            }

            // ----- pulo da goleira (mesma fisica do Rico) -----
            if (goleiroVelY != 0) {
                goleiroY = goleiroY + goleiroVelY;
                goleiroVelY = goleiroVelY + 0.8;
                if (goleiroY >= CHAO - GOLEIRO_A) {
                    goleiroY = CHAO - GOLEIRO_A;
                    goleiroVelY = 0;
                }
            }

            // ----- tempo da mensagem de resultado -----
            if (resultado != NULL) {
                tempoResultado--;
                if (tempoResultado <= 0) {
                    resultado = NULL;
                    reiniciar = true;
                    if (gols >= 3) estado = VITORIA;
                    else if (arremessos >= MAX_TENTATIVAS) estado = DERROTA;   // usou as 7 tentativas
                }
            }
        }

        // =============================================
        // 3) DESENHAR
        // so desenha quando nao tem mais eventos esperando (evita travar)
        // =============================================
        if (rodando && redesenhar && al_is_event_queue_empty(fila)) {

            redesenhar = false;

            if (estado == REGRAS) {
                char* regras[5] = { "1. Com a bola na mao, no maximo 3 passos.",
                    "2. Aperte S para quicar a bola e ganhar mais 3 passos.",
                    "3. Nao pise na area do goleiro segurando a bola.",
                    "4. Mas pode pular na area e arremessar antes de cair!",
                    "5. A barra de forca define a altura do arremesso." };
                tela_regras("HANDEBOL", regras, "Faca 3 gols em 7 tentativas para passar de fase.", bola, 80, 80);
            }
            else if (estado == VITORIA || estado == DERROTA) {
                sprintf(texto, "Gols: %d / 3  |  Arremessos: %d / %d", gols, arremessos, MAX_TENTATIVAS);
                if (estado == VITORIA) sprintf(texto2, "Pontos da fase: %d  (30 - 2 por arremesso errado)", calcular_pontos(arremessos - gols, 2));
                else sprintf(texto2, "Voce precisa fazer 3 gols em 7 tentativas.");
                tela_fim(estado == VITORIA, texto, texto2);
            }
            else {
                // fundo, chao em tons suaves e area do goleiro um pouco mais escura
                al_clear_to_color(al_map_rgb(215, 235, 245));
                al_draw_filled_rectangle(0, CHAO, AREA, ALTURA, al_map_rgb(176, 196, 214));
                al_draw_filled_rectangle(AREA, CHAO, LARGURA, ALTURA, al_map_rgb(143, 168, 192));
                al_draw_line(0, CHAO, LARGURA, CHAO, branco, 4);
                al_draw_line(AREA, CHAO, AREA, CHAO + 45, branco, 5);

                // gol: trave (em pe) e travessao (deitado)
                al_draw_filled_rectangle(GOL_X - 8, GOL_Y, GOL_X, CHAO, al_map_rgb(220, 30, 30));
                al_draw_filled_rectangle(GOL_X - 8, GOL_Y - 8, LARGURA, GOL_Y, al_map_rgb(220, 30, 30));

                // placar e barra de forca
                sprintf(texto, "Gols: %d / 3    Passos: %d / 3    Tentativas: %d / %d", gols, passos, arremessos, MAX_TENTATIVAS);
                escrever(fonte, al_map_rgb(0, 0, 0), 20, texto);
                desenhar_barra(forca);

                // goleira: a imagem do pulo olha para a direita, entao espelho
                // andando, ela fica espelhada quando vai para a esquerda
                if (goleiroVelY != 0) desenhar(anaPulando, goleiroX, goleiroY, GOLEIRO_L, GOLEIRO_A, ALLEGRO_FLIP_HORIZONTAL);
                else desenhar(anaAndando, goleiroX, goleiroY, GOLEIRO_L, GOLEIRO_A, goleiroVelX < 0 ? ALLEGRO_FLIP_HORIZONTAL : 0);

                // Rico e a bola (na mao ou voando)
                desenhar_rico(ricoX, ricoY, direcao, noChao, tempoAndando);
                if (comBola) desenhar_bola_na_mao(ricoX, ricoY, direcao, noChao, tempoDrible);
                else if (resultado == NULL) desenhar(bola, bolaX, bolaY, BOLA_T, BOLA_T, 0);

                char* teclas[6] = { "A", "D", "W", "S", "ESPACO", "ESC" };
                char* acoes[6] = { NULL, "andar", "pular", "quicar", "arremessar", "menu" };
                desenhar_legenda(6, teclas, acoes);
                if (resultado != NULL) escrever(fonteGrande, al_map_rgb(255, 140, 0), 200, resultado);

                // aviso de infracao
                if (infracao == 1) caixa_aviso("INFRACAO!", "Mais de 3 passos segurando a bola.", "Quique a bola (S) para andar mais.");
                if (infracao == 2) caixa_aviso("INFRACAO!", "Voce pisou na area com a bola.", "Pule e arremesse antes de cair!");
            }

            al_flip_display();
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    return retorno;
}
