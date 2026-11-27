// =====================================================
// DuoFit - Fase de Basquete
// Mesmas teclas do handebol: A/D andar, W pular, S quicar, ESPACO arremessar.
// =====================================================

#include "duofit.h"

// estados da fase: cada numero e uma tela diferente
#define REGRAS   1
#define JOGO     2
#define VITORIA  3
#define DERROTA  4    // acabaram as tentativas: refaz a fase

// posicoes da quadra (CHAO, RICO_L, RICO_A e BOLA_T ficam no duofit.h)
#define LINHA_3    470    // x da linha de 3 pontos
#define RICO_MAX_X 750    // o Rico nao passa daqui (embaixo da cesta nao da)
#define ARO_Y      300    // altura do aro
#define ARO_X1     990    // comeco do aro
#define ARO_X2     1070   // fim do aro (encostado na tabela)
#define TABELA_X   1072   // frente da tabela
#define TABELA_Y1  160    // topo da tabela
#define TABELA_Y2  310    // base da tabela

#define META_PONTOS 10   // pontos para passar de fase
#define MAX_PASSOS  2    // no basquete sao 2 passos com a bola na mao

int jogar_basquete(void) {

    int retorno = VOLTAR_MENU;
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* fila = criar_fila(timer);

    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR preto = al_map_rgb(0, 0, 0);

    // ---------- variaveis do jogo ----------
    int estado = REGRAS;

    // Rico
    float ricoX = 150, ricoY = CHAO - RICO_A, ricoVelY = 0;
    int direcao = 1;             // 1 = direita, -1 = esquerda
    int tempoAndando = 0;
    bool noChao = true;
    bool jaMexeuNoAr = false;

    // bola
    bool comBola = true;
    int passos = 0;
    int tempoDrible = 0;
    float bolaX = 0, bolaY = 0, bolaVelX = 0, bolaVelY = 0;
    float forca = 0;             // barra de forca de 0 a 100
    float variacaoForca = 2;
    bool deTres = false;         // o arremesso saiu de tras da linha de 3?
    bool voltando = false;       // a bola ja quicou no chao e esta voltando para o Rico?

    // placar
    int pontos = 0, arremessos = 0, cestas = 0;
    bool andada = false;         // mais de 2 passos com a bola
    char* resultado = NULL;
    int tempoResultado = 0;
    bool reiniciar = true;

    bool rodando = true, redesenhar = false;
    char texto[100], texto2[100];

    al_start_timer(timer);

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
                pontos = 0; arremessos = 0; cestas = 0; andada = false; resultado = NULL;
                reiniciar = true;
                estado = JOGO;
            }
            // ultima fase: depois da pergunta volta para o menu
            else if (estado == VITORIA && tecla == ALLEGRO_KEY_ENTER) {
                char* opcoes[3] = { "1 ponto", "2 pontos", "3 pontos" };
                retorno = fazer_pergunta("BASQUETE",
                    "Quanto vale uma cesta feita de tras da linha de 3 pontos?", opcoes, 2,
                    "Vale 3 pontos. Dentro da linha, a cesta vale 2.",
                    calcular_pontos(arremessos - cestas, 2), true);
                rodando = false;
            }
            // com o aviso de andada aberto, so o ENTER funciona
            else if (estado == JOGO && andada) {
                if (tecla == ALLEGRO_KEY_ENTER) andada = false;
            }
            else if (estado == JOGO && resultado == NULL) {

                // A/D andar, W pular, S quicar (igual ao handebol, a funcao esta no comum.c)
                if (mover_rico(tecla, MAX_PASSOS, comBola, &ricoX, &ricoVelY, &direcao, &noChao,
                    &jaMexeuNoAr, &tempoAndando, &passos, &tempoDrible)) {
                    andada = true;   // mais de 2 passos com a bola
                }

                // ESPACO = arremessar
                if (tecla == ALLEGRO_KEY_SPACE && comBola && tempoDrible == 0) {
                    bolaX = ricoX + (direcao == 1 ? 105 : -5);
                    bolaY = ricoY + (noChao ? 100 : 20);

                    // a bola sai bem inclinada para cima (uns 67 graus)
                    // 0.39 e 0.92 sao o cosseno e o seno desse angulo
                    // mais forca = mais velocidade = vai mais longe
                    float velocidade = 14 + forca * 0.12;
                    bolaVelX = velocidade * 0.39 * direcao;
                    bolaVelY = -velocidade * 0.92;

                    // se o meio do Rico esta antes da linha, vale 3
                    deTres = (ricoX + RICO_L / 2 <= LINHA_3);
                    comBola = false;
                    arremessos++;
                }
            }
        }

        // =============================================
        // 2) ATUALIZAR
        // =============================================
        if (evento.type == ALLEGRO_EVENT_TIMER) redesenhar = true;

        if (evento.type == ALLEGRO_EVENT_TIMER && estado == JOGO) {

            if (reiniciar) {
                ricoX = 150; ricoY = CHAO - RICO_A; ricoVelY = 0; noChao = true; direcao = 1;
                comBola = true; passos = 0; tempoDrible = 0;
                reiniciar = false;
            }

            // pulo do Rico (igual ao handebol)
            atualizar_pulo(&ricoY, &ricoVelY, &noChao);

            // nao sai da tela e nao entra embaixo da cesta
            if (ricoX < 0) ricoX = 0;
            if (ricoX > RICO_MAX_X) ricoX = RICO_MAX_X;

            if (tempoAndando > 0) tempoAndando--;
            if (tempoDrible > 0) tempoDrible--;

            // barra de forca
            if (comBola) {
                forca = forca + variacaoForca;
                if (forca >= 100 || forca <= 0) variacaoForca = -variacaoForca;
            }

            // ----- bola voando -----
            if (!comBola && resultado == NULL) {

                float meioAntesY = bolaY + BOLA_T / 2;   // meio da bola no frame anterior

                bolaX = bolaX + bolaVelX;
                bolaY = bolaY + bolaVelY;
                bolaVelY = bolaVelY + 0.5;

                float meioX = bolaX + BOLA_T / 2;
                float meioY = bolaY + BOLA_T / 2;

                // bateu na tabela: volta mais devagar (pode entrar de tabela)
                if (bolaVelX > 0 && bolaX + BOLA_T >= TABELA_X && meioY >= TABELA_Y1 && meioY <= TABELA_Y2) {
                    bolaX = TABELA_X - BOLA_T;
                    bolaVelX = -bolaVelX * 0.6;
                }

                // cesta: o meio da bola passa pela altura do aro DESCENDO e dentro do aro
                if (bolaVelY > 0 && meioAntesY < ARO_Y && meioY >= ARO_Y &&
                    meioX >= ARO_X1 + 8 && meioX <= ARO_X2 - 8) {
                    resultado = deTres ? "CESTA DE 3!" : "CESTA DE 2!";
                    pontos = pontos + (deTres ? 3 : 2);
                    cestas++;
                    bolaVelX = 0;   // depois da cesta a bola cai reto pela rede
                    tocar_som_gol();
                }
                else if (bolaY + BOLA_T >= CHAO) resultado = "ERROU!";
                else if (bolaX > LARGURA || bolaX < -BOLA_T) resultado = "PARA FORA!";

                if (resultado != NULL) {
                    tempoResultado = 120;   // 2 segundos: da tempo da bola voltar
                    voltando = false;
                }
            }

            // tempo da mensagem
            if (resultado != NULL) {

                // enquanto a mensagem aparece, a bola quica no chao e volta para o Rico
                bolaY = bolaY + bolaVelY;
                bolaVelY = bolaVelY + 0.5;
                if (bolaY + BOLA_T >= CHAO) {
                    bolaY = CHAO - BOLA_T;
                    bolaVelY = -bolaVelY * 0.75;   // cada quique sobe um pouco menos
                    voltando = true;
                }

                if (!voltando) bolaX = bolaX + bolaVelX;
                else {
                    // anda uma fracao do que falta para chegar na mao quando o tempo acabar
                    float maoX = ricoX + (direcao == 1 ? 102 : -3);
                    bolaX = bolaX + (maoX - bolaX) / tempoResultado;
                    // nos ultimos 20 frames a bola sobe ate a mao
                    if (tempoResultado <= 20) bolaY = bolaY + (ricoY + 100 - bolaY) / tempoResultado;
                }

                tempoResultado--;
                if (tempoResultado <= 0) {
                    resultado = NULL;
                    reiniciar = true;
                    if (pontos >= META_PONTOS) estado = VITORIA;
                    else if (arremessos >= MAX_TENTATIVAS) estado = DERROTA;   // usou as 7 tentativas
                }
            }
        }

        // =============================================
        // 3) DESENHAR
        // =============================================
        if (rodando && redesenhar && al_is_event_queue_empty(fila)) {

            redesenhar = false;

            if (estado == REGRAS) {
                char* regras[5] = { "1. Com a bola na mao, no maximo 2 passos (senao e ANDADA).",
                    "2. Aperte S para quicar a bola e ganhar mais 2 passos.",
                    "3. A barra de forca define a distancia do arremesso.",
                    "4. De tras da linha branca, a cesta vale 3 pontos!",
                    "5. Pode pular (W) e arremessar no ar. Bola na tabela vale!" };
                tela_regras("BASQUETE", regras, "Faca 10 pontos em 7 tentativas para passar de fase.", bola, 80, 80);
            }
            else if (estado == VITORIA || estado == DERROTA) {
                sprintf(texto, "Pontos: %d / %d  |  Cestas: %d  |  Arremessos: %d / %d", pontos, META_PONTOS, cestas, arremessos, MAX_TENTATIVAS);
                if (estado == VITORIA) sprintf(texto2, "Pontos da fase: %d  (30 - 2 por arremesso errado)", calcular_pontos(arremessos - cestas, 2));
                else sprintf(texto2, "Voce precisa fazer 10 pontos em 7 tentativas.");
                tela_fim(estado == VITORIA, texto, texto2);
            }
            else {
                // quadra com as cores do handebol
                al_clear_to_color(al_map_rgb(215, 235, 245));
                al_draw_filled_rectangle(0, CHAO, LARGURA, ALTURA, al_map_rgb(40, 90, 170));
                al_draw_filled_rectangle(900, CHAO, LARGURA, ALTURA, al_map_rgb(240, 140, 40));   // garrafao

                // linha de 3 pontos
                al_draw_line(LINHA_3, CHAO, LINHA_3, CHAO + 45, branco, 5);
                al_draw_text(fonte, branco, LINHA_3 - 10, CHAO + 8, ALLEGRO_ALIGN_RIGHT, "3 PONTOS");

                // cesta: poste, braco, tabela, aro e rede
                al_draw_filled_rectangle(1150, 200, 1162, CHAO, al_map_rgb(90, 90, 90));
                al_draw_filled_rectangle(1088, 225, 1156, 235, al_map_rgb(90, 90, 90));
                al_draw_filled_rectangle(TABELA_X, TABELA_Y1, TABELA_X + 16, TABELA_Y2, branco);
                al_draw_rectangle(TABELA_X, TABELA_Y1, TABELA_X + 16, TABELA_Y2, preto, 3);
                al_draw_line(ARO_X1, ARO_Y, TABELA_X, ARO_Y, al_map_rgb(220, 30, 30), 6);
                for (int i = 0; i <= 4; i++) {
                    al_draw_line(ARO_X1 + 5 + i * 18, ARO_Y + 3, ARO_X1 + 15 + i * 13, ARO_Y + 50, branco, 2);
                }

                // placar e barra de forca
                sprintf(texto, "Pontos: %d / %d    Passos: %d / %d    Tentativas: %d / %d",
                    pontos, META_PONTOS, passos, MAX_PASSOS, arremessos, MAX_TENTATIVAS);
                escrever(fonte, preto, 20, texto);
                desenhar_barra(forca);

                // Rico e a bola (na mao ou voando)
                desenhar_rico(ricoX, ricoY, direcao, noChao, tempoAndando);
                if (comBola) desenhar_bola_na_mao(ricoX, ricoY, direcao, noChao, tempoDrible);
                else desenhar(bola, bolaX, bolaY, BOLA_T, BOLA_T, 0);

                char* teclas[6] = { "A", "D", "W", "S", "ESPACO", "ESC" };
                char* acoes[6] = { NULL, "andar", "pular", "quicar", "arremessar", "menu" };
                desenhar_legenda(6, teclas, acoes);
                if (resultado != NULL) escrever(fonteGrande, al_map_rgb(255, 140, 0), 200, resultado);

                // aviso de andada (igual ao de infracao do handebol)
                if (andada) caixa_aviso("ANDADA!", "Mais de 2 passos segurando a bola.", "Quique a bola (S) para andar mais.");
            }

            al_flip_display();
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    return retorno;
}
