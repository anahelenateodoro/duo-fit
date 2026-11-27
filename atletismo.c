// =====================================================
// DuoFit - Fase de Atletismo (revezamento com barreiras)
// Ana corre a primeira parte, passa o bastao e o Rico termina.
// =====================================================

#include "duofit.h"

// estados da fase: cada numero e uma tela diferente
#define REGRAS   1
#define CORRIDA  2
#define VITORIA  3
#define DERROTA  4    // derrubou uma barreira: refaz a fase

// pista: 40 pixels = 1 metro, entao 100 metros = 4000 pixels
#define INICIO        150
#define CHEGADA       4150
#define FIM_DO_MUNDO  4500   // ate onde a camera pode ir
#define TROCA_INICIO  1950   // zona de troca do bastao (45 m a 55 m)
#define TROCA_FIM     2350
#define TROCA_X       2150   // onde o Rico fica esperando

// velocidades em pixels por frame (60 frames por segundo)
#define VEL_MAXIMA    8.5
#define ACELERACAO    0.07   // segurando D
#define FREIO         0.1    // soltando D

#define MAX_DERRUBADAS 0     // nao pode derrubar nenhuma barreira

int jogar_atletismo(void) {

    int retorno = VOLTAR_MENU;
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* fila = criar_fila(timer);

    // ---------- cores (as mesmas do handebol) ----------
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR preto = al_map_rgb(0, 0, 0);
    ALLEGRO_COLOR laranja = al_map_rgb(255, 140, 0);
    ALLEGRO_COLOR vermelho = al_map_rgb(220, 30, 30);

    // ---------- barreiras ----------
    int barreiras[6] = { 700, 1250, 1750, 2700, 3250, 3800 };   // posicao de cada uma
    bool derrubada[6];

    // ---------- variaveis da corrida ----------
    // (os valores iniciais sao colocados quando aperta ENTER na tela de regras)
    int estado = REGRAS;
    float posicao = INICIO;       // onde o corredor esta na pista
    float velocidade = 0;         // pixels por frame
    float camera = 0;             // quanto o cenario andou para a esquerda
    float altura = 0, velPulo = 0;   // altura do pulo (0 = no chao) e velocidade do pulo
    int atleta = 0;               // 0 = Ana, 1 = Rico
    int derrubadas = 0, frames = 0;  // barreiras derrubadas e cronometro em frames (60 = 1 segundo)
    int tempoAviso = 0;           // frames mostrando "BARREIRA DERRUBADA!"
    int tempoChegada = 0;         // frames mostrando "CHEGOU!"
    int quadro = 0;               // 0 ou 1: qual das duas poses de corrida aparece
    int contaAnimacao = 0;        // frames desde a ultima troca de pose
    bool segurandoD = false, terminou = false;

    bool rodando = true, redesenhar = false;
    char texto[120], texto2[120];

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

            // regras ou derrota: ENTER comeca a corrida do zero
            if ((estado == REGRAS || estado == DERROTA) && tecla == ALLEGRO_KEY_ENTER) {
                for (int i = 0; i < 6; i++) derrubada[i] = false;
                posicao = INICIO; velocidade = 0; camera = 0; altura = 0; velPulo = 0;
                atleta = 0; derrubadas = 0; frames = 0; tempoAviso = 0; tempoChegada = 0;
                quadro = 0; contaAnimacao = 0; segurandoD = false; terminou = false;
                estado = CORRIDA;
            }
            // vitoria: libera o basquete no menu e abre a pergunta do final
            else if (estado == VITORIA && tecla == ALLEGRO_KEY_ENTER) {
                if (faseLiberada < 3) faseLiberada = 3;
                char* opcoes[3] = { "Em qualquer ponto da pista",
                    "Dentro da zona de troca", "Depois da linha de chegada" };
                retorno = fazer_pergunta("ATLETISMO",
                    "No revezamento, onde deve acontecer a passagem do bastao?", opcoes, 1,
                    "A passagem so vale dentro da zona de troca.",
                    calcular_pontos(derrubadas, 5), false);
                rodando = false;
            }
            else if (estado == CORRIDA && !terminou) {
                // D = correr (segurando)
                if (tecla == ALLEGRO_KEY_D) segurandoD = true;
                // W = pular, so se estiver no chao
                if (tecla == ALLEGRO_KEY_W && altura == 0) velPulo = 11;
                // ESPACO = passar o bastao (Ana no chao, dentro da zona)
                if (tecla == ALLEGRO_KEY_SPACE && atleta == 0 && altura == 0 &&
                    posicao >= TROCA_INICIO && posicao <= TROCA_FIM) {
                    atleta = 1;
                    posicao = TROCA_X;
                    quadro = 0;
                }
            }
        }
        if (evento.type == ALLEGRO_EVENT_KEY_UP && evento.keyboard.keycode == ALLEGRO_KEY_D) segurandoD = false;

        // =============================================
        // 2) ATUALIZAR (a cada tique do timer)
        // =============================================
        if (evento.type == ALLEGRO_EVENT_TIMER) redesenhar = true;

        if (evento.type == ALLEGRO_EVENT_TIMER && estado == CORRIDA) {

            if (!terminou) {
                // ----- velocidade: segurando D acelera, soltando freia -----
                if (segurandoD) velocidade = velocidade + ACELERACAO;
                else velocidade = velocidade - FREIO;
                if (velocidade > VEL_MAXIMA) velocidade = VEL_MAXIMA;
                if (velocidade < 0) velocidade = 0;

                // ----- andar -----
                float posicaoAntes = posicao;
                posicao = posicao + velocidade;

                // Ana espera no fim da zona se nao passou o bastao
                if (atleta == 0 && posicao >= TROCA_FIM) {
                    posicao = TROCA_FIM;
                    velocidade = 0;
                }

                // chegou
                if (posicao >= CHEGADA) {
                    posicao = CHEGADA;
                    velocidade = 0;
                    terminou = true;
                    tocar_som_gol();
                }

                // cronometro comeca quando sai do lugar
                if (posicao > INICIO && !terminou) frames++;

                // ----- pulo (mesma ideia do handebol, mas a altura cresce para cima) -----
                if (altura > 0 || velPulo > 0) {
                    altura = altura + velPulo;
                    velPulo = velPulo - 0.45;
                    if (altura <= 0) {
                        altura = 0;
                        velPulo = 0;
                    }
                }

                // ----- barreiras -----
                // testo o pedaco que andou nesse frame para nao "pular" a barreira
                for (int i = 0; i < 6; i++) {
                    if (!derrubada[i] && posicaoAntes <= barreiras[i] + 22 &&
                        posicao >= barreiras[i] - 22 && altura < 85) {
                        derrubada[i] = true;
                        derrubadas++;
                        velocidade = velocidade * 0.4;   // perde velocidade
                        tempoAviso = 90;
                    }
                }
                if (tempoAviso > 0) tempoAviso--;

                // derrubou barreira: o corredor para, mostra o aviso por 1,5 s e perde a fase
                if (derrubadas > MAX_DERRUBADAS) {
                    velocidade = 0;
                    segurandoD = false;
                    if (tempoAviso == 0) estado = DERROTA;
                }

                // ----- animacao: troca de pose a cada 7 frames -----
                if (velocidade > 0) {
                    contaAnimacao++;
                    if (contaAnimacao >= 7) {
                        contaAnimacao = 0;
                        quadro = 1 - quadro;   // 0 vira 1 e 1 vira 0
                    }
                }
                else {
                    contaAnimacao = 0;
                    quadro = 0;
                }

                // ----- camera segue o corredor -----
                camera = posicao - 350;
                if (camera < 0) camera = 0;
                if (camera > FIM_DO_MUNDO - LARGURA) camera = FIM_DO_MUNDO - LARGURA;
            }
            else {
                // depois de chegar, mostra "CHEGOU!" por 1,5 s
                // so passa de fase se pulou a maioria das barreiras
                tempoChegada++;
                if (tempoChegada >= 90) {
                    if (derrubadas <= MAX_DERRUBADAS) estado = VITORIA;
                    else estado = DERROTA;
                }
            }
        }

        // =============================================
        // 3) DESENHAR
        // =============================================
        if (rodando && redesenhar && al_is_event_queue_empty(fila)) {

            redesenhar = false;

            if (estado == REGRAS) {
                char* regras[5] = { "1. Segure D para correr. Soltando, o atleta desacelera.",
                    "2. Aperte W para pular as barreiras.",
                    "3. Se bater em uma barreira, voce perde e refaz a fase.",
                    "4. Na zona laranja, aperte ESPACO para passar o bastao ao Rico.",
                    "5. Ana corre a primeira parte e Rico termina a corrida." };
                tela_regras("ATLETISMO", regras, "Chegue aos 100 m sem derrubar nenhuma barreira.", anaCorrendo, 120, 180);
            }
            else if (estado == VITORIA || estado == DERROTA) {
                sprintf(texto, "Tempo: %.2f s  |  Barreiras derrubadas: %d de 6", frames / 60.0, derrubadas);
                if (estado == VITORIA) sprintf(texto2, "Pontos da fase: %d  (30 - 5 por barreira derrubada)", calcular_pontos(derrubadas, 5));
                else sprintf(texto2, "Voce nao pode derrubar nenhuma barreira. Tente de novo!");
                tela_fim(estado == VITORIA, texto, texto2);
            }
            else {
                // fundo claro e pista azul (cores da quadra do handebol)
                al_clear_to_color(al_map_rgb(215, 235, 245));
                al_draw_filled_rectangle(0, 530, LARGURA, ALTURA, al_map_rgb(40, 90, 170));
                for (int i = 0; i <= 4; i++) {
                    al_draw_line(0, 530 + i * 40, LARGURA, 530 + i * 40, branco, 3);   // raias
                }

                // marcas a cada 10 metros
                // tudo que fica na pista e desenhado em (posicao - camera)
                for (int m = 0; m <= 100; m = m + 10) {
                    float x = INICIO + m * 40 - camera;
                    al_draw_line(x, 535, x, 685, branco, 2);
                    sprintf(texto, "%d m", m);
                    al_draw_text(fonte, branco, x + 5, 650, 0, texto);
                }

                // linha de chegada xadrez: quadradinhos pretos e brancos alternados
                float chegadaX = CHEGADA - camera;
                for (int y = 530; y < 690; y = y + 20) {
                    for (int j = 0; j < 2; j++) {
                        al_draw_filled_rectangle(chegadaX + j * 20, y, chegadaX + j * 20 + 20, y + 20,
                            (y / 20 + j) % 2 == 0 ? preto : branco);
                    }
                }
                al_draw_text(fonte, preto, chegadaX + 20, 385, ALLEGRO_ALIGN_CENTER, "CHEGADA");

                // zona de troca em laranja (como a area do handebol)
                al_draw_filled_rectangle(TROCA_INICIO - camera, 590, TROCA_FIM - camera, 630, al_map_rgb(240, 140, 40));
                al_draw_text(fonte, branco, TROCA_X - camera + 60, 597, ALLEGRO_ALIGN_CENTER, "ZONA DE TROCA");

                // o parceiro que nao esta correndo fica parado esperando
                if (atleta == 0) desenhar(ricoParado, TROCA_X - camera - 70, 380, 140, 210, 0);
                else desenhar(anaParada, TROCA_INICIO - camera - 70, 380, 140, 210, 0);

                // barreiras: em pe ou caida no chao
                for (int i = 0; i < 6; i++) {
                    float x = barreiras[i] - camera;
                    if (derrubada[i]) {
                        al_draw_filled_rectangle(x - 35, 622, x + 35, 630, vermelho);
                    }
                    else {
                        al_draw_line(x - 25, 630, x - 25, 550, preto, 5);
                        al_draw_line(x + 25, 630, x + 25, 550, preto, 5);
                        al_draw_filled_rectangle(x - 32, 542, x + 32, 553, vermelho);
                    }
                }

                // corredor: escolhe a imagem certa
                // correndo, troca entre duas poses para parecer que esta andando
                ALLEGRO_BITMAP* imagem;
                if (altura > 0) imagem = (atleta == 0) ? anaPulando : ricoPulando;
                else if (velocidade == 0) imagem = (atleta == 0) ? anaParada : ricoParado;
                else if (atleta == 0) imagem = (quadro == 0) ? anaAndando : anaCorrendo;
                else imagem = (quadro == 0) ? ricoAndando : ricoParado;
                desenhar(imagem, posicao - camera - 70, 420 - altura, 140, 210, 0);

                // bastao amarelo na mao de quem esta correndo
                float maoX = posicao - camera + 35;
                al_draw_line(maoX, 520 - altura, maoX + 25, 505 - altura, al_map_rgb(255, 215, 20), 7);

                // placar
                sprintf(texto, "Distancia: %.0f / 100 m    Tempo: %.2f s",
                    (posicao - INICIO) / 40, frames / 60.0);
                escrever(fonte, preto, 20, texto);
                escrever(fonte, preto, 55, atleta == 0 ? "Correndo: Ana" : "Correndo: Rico");

                // barra de velocidade (no lugar da barra de forca do handebol)
                desenhar_barra(velocidade / VEL_MAXIMA * 100);

                // mensagens grandes
                if (terminou) {
                    escrever(fonteGrande, laranja, 200, "CHEGOU!");
                }
                else if (tempoAviso > 0) {
                    escrever(fonteGrande, laranja, 200, "BARREIRA DERRUBADA!");
                }
                else if (atleta == 0 && posicao >= TROCA_INICIO) {
                    escrever(fonteGrande, laranja, 200, "PASSE O BASTAO!");
                    escrever(fonte, preto, 270, "Aperte ESPACO dentro da zona laranja");
                }
                else if (posicao < barreiras[0]) {
                    escrever(fonte, preto, 200, "Segure D para correr e aperte W para pular a barreira");
                }

                char* teclas[4] = { "D", "W", "ESPACO", "ESC" };
                char* acoes[4] = { "correr", "pular", "passar o bastao", "menu" };
                desenhar_legenda(4, teclas, acoes);
            }

            al_flip_display();
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    return retorno;
}
