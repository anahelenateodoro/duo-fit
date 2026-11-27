// DuoFit - funcoes usadas por todas as fases

#include "duofit.h"

// variaveis globais (declaradas com extern no duofit.h)
ALLEGRO_FONT* fonte = NULL;
ALLEGRO_FONT* fonteGrande = NULL;
ALLEGRO_FONT* fonteMenu = NULL;
ALLEGRO_FONT* fonteGiz = NULL;
ALLEGRO_FONT* fonteGizTitulo = NULL;

ALLEGRO_BITMAP* imagemAna = NULL;    // folha com todas as poses da Ana
ALLEGRO_BITMAP* imagemRico = NULL;   // folha com todas as poses do Rico
ALLEGRO_BITMAP* bola = NULL;
ALLEGRO_BITMAP* anaParada = NULL;
ALLEGRO_BITMAP* anaAndando = NULL;
ALLEGRO_BITMAP* anaCorrendo = NULL;
ALLEGRO_BITMAP* anaPulando = NULL;
ALLEGRO_BITMAP* ricoParado = NULL;
ALLEGRO_BITMAP* ricoAndando = NULL;
ALLEGRO_BITMAP* ricoPulando = NULL;
ALLEGRO_BITMAP* fotoAna = NULL;    // fotos do menu
ALLEGRO_BITMAP* fotoRico = NULL;

ALLEGRO_SAMPLE* somGol = NULL;   // se ficar NULL o jogo roda sem som

int pontosTotal = 0;
int pontosPorFase[3] = { 0, 0, 0 };
int recorde = 0;
int faseLiberada = 1;   // no comeco so o handebol esta liberado

// Tenta abrir a Arial do Windows, depois a do Mac.
// Se nao achar nenhuma usa a fonte que ja vem no Allegro.
ALLEGRO_FONT* abrir_fonte(int tamanho, bool negrito) {
    ALLEGRO_FONT* f = al_load_ttf_font(negrito ? "C:/Windows/Fonts/arialbd.ttf" : "C:/Windows/Fonts/arial.ttf", tamanho, 0);
    if (f == NULL) f = al_load_ttf_font(negrito ? "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
                                                : "/System/Library/Fonts/Supplemental/Arial.ttf", tamanho, 0);
    if (f == NULL) f = al_create_builtin_font();
    return f;
}

// Fonte com cara de giz (Comic Sans do Windows ou do Mac).
// Se nao achar usa a mesma fonte do resto do jogo.
ALLEGRO_FONT* abrir_fonte_giz(int tamanho, bool negrito) {
    ALLEGRO_FONT* f = al_load_ttf_font(negrito ? "C:/Windows/Fonts/comicbd.ttf" : "C:/Windows/Fonts/comic.ttf", tamanho, 0);
    if (f == NULL) f = al_load_ttf_font(negrito ? "/System/Library/Fonts/Supplemental/Comic Sans MS Bold.ttf"
                                                : "/System/Library/Fonts/Supplemental/Comic Sans MS.ttf", tamanho, 0);
    if (f == NULL) f = abrir_fonte(tamanho, negrito);
    return f;
}

// Carrega tudo do jogo. Devolve false se faltar alguma imagem.
bool carregar_recursos(void) {
    fonte = abrir_fonte(24, false);
    fonteGrande = abrir_fonte(56, true);
    fonteMenu = abrir_fonte(32, false);
    fonteGiz = abrir_fonte_giz(27, false);
    fonteGizTitulo = abrir_fonte_giz(60, true);

    imagemAna = al_load_bitmap("assets/ana_sprites.png");
    imagemRico = al_load_bitmap("assets/rico_sprites.png");
    bola = al_load_bitmap("assets/bola.png");
    if (imagemAna == NULL || imagemRico == NULL || bola == NULL) {
        return false;
    }

    // Recorta cada pose da folha de imagens.
    // al_create_sub_bitmap nao copia a imagem, so aponta para um pedaco dela.
    // Ana: cada pose tem 256 x 384 (3 na linha de cima, 2 na de baixo)
    anaParada = al_create_sub_bitmap(imagemAna, 0, 0, 256, 384);
    anaAndando = al_create_sub_bitmap(imagemAna, 256, 0, 256, 384);
    anaCorrendo = al_create_sub_bitmap(imagemAna, 0, 384, 256, 384);
    anaPulando = al_create_sub_bitmap(imagemAna, 256, 384, 256, 384);
    // Rico: as 3 poses de 256 x 384 ficam na linha de cima
    ricoParado = al_create_sub_bitmap(imagemRico, 0, 0, 256, 384);
    ricoAndando = al_create_sub_bitmap(imagemRico, 256, 0, 256, 384);
    ricoPulando = al_create_sub_bitmap(imagemRico, 512, 0, 256, 384);

    // fotos do menu: se tiver foto_ana.png e foto_rico.png usa elas,
    // se nao tiver usa a pose parada de cada personagem
    fotoAna = al_load_bitmap("assets/foto_ana.png");
    fotoRico = al_load_bitmap("assets/foto_rico.png");
    if (fotoAna == NULL) fotoAna = anaParada;
    if (fotoRico == NULL) fotoRico = ricoParado;

    // som do gol (se nao tiver placa de som ou o arquivo, segue sem som)
    if (al_install_audio() && al_init_acodec_addon() && al_reserve_samples(1)) {
        somGol = al_load_sample("assets/gol.wav");
    }
    if (somGol == NULL) {
        printf("Aviso: o jogo vai rodar sem som.\n");
    }
    return true;
}

// Libera a memoria no fim do jogo.
// Primeiro os recortes, depois as imagens grandes.
void liberar_recursos(void) {
    if (anaParada != NULL) al_destroy_bitmap(anaParada);
    if (anaAndando != NULL) al_destroy_bitmap(anaAndando);
    if (anaCorrendo != NULL) al_destroy_bitmap(anaCorrendo);
    if (anaPulando != NULL) al_destroy_bitmap(anaPulando);
    if (ricoParado != NULL) al_destroy_bitmap(ricoParado);
    if (ricoAndando != NULL) al_destroy_bitmap(ricoAndando);
    if (ricoPulando != NULL) al_destroy_bitmap(ricoPulando);
    if (imagemAna != NULL) al_destroy_bitmap(imagemAna);
    if (imagemRico != NULL) al_destroy_bitmap(imagemRico);
    if (bola != NULL) al_destroy_bitmap(bola);
    // so destroi as fotos se vieram de arquivo (senao sao as poses, ja destruidas acima)
    if (fotoAna != NULL && fotoAna != anaParada) al_destroy_bitmap(fotoAna);
    if (fotoRico != NULL && fotoRico != ricoParado) al_destroy_bitmap(fotoRico);
    if (somGol != NULL) {
        al_stop_samples();
        al_destroy_sample(somGol);
    }
    if (fonte != NULL) al_destroy_font(fonte);
    if (fonteGrande != NULL) al_destroy_font(fonteGrande);
    if (fonteMenu != NULL) al_destroy_font(fonteMenu);
    if (fonteGiz != NULL) al_destroy_font(fonteGiz);
    if (fonteGizTitulo != NULL) al_destroy_font(fonteGizTitulo);
}

// Escreve um texto centralizado na horizontal, na altura y.
void escrever(ALLEGRO_FONT* f, ALLEGRO_COLOR cor, float y, char* mensagem) {
    al_draw_text(f, cor, LARGURA / 2, y, ALLEGRO_ALIGN_CENTER, mensagem);
}

// Desenha a imagem na posicao (x, y) com a largura e altura que eu quiser.
// A imagem original pode ter outro tamanho: o Allegro estica ou encolhe.
// flags = 0 desenha normal, ALLEGRO_FLIP_HORIZONTAL desenha espelhado.
void desenhar(ALLEGRO_BITMAP* imagem, float x, float y, float largura, float altura, int flags) {
    float larguraOriginal = al_get_bitmap_width(imagem);
    float alturaOriginal = al_get_bitmap_height(imagem);
    al_draw_scaled_bitmap(imagem, 0, 0, larguraOriginal, alturaOriginal, x, y, largura, altura, flags);
}

// Toca o som de comemoracao (gol, cesta e chegada).
void tocar_som_gol(void) {
    if (somGol != NULL) {
        al_stop_samples();   // para o som anterior se ainda estiver tocando
        al_play_sample(somGol, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
}

// Cria a fila de eventos e faz ela "escutar" a janela, o teclado e o timer.
// O menu e a pergunta nao usam timer: nesse caso passa NULL.
ALLEGRO_EVENT_QUEUE* criar_fila(ALLEGRO_TIMER* timer) {
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    al_register_event_source(fila, al_get_display_event_source(al_get_current_display()));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_mouse_event_source());
    if (timer != NULL) al_register_event_source(fila, al_get_timer_event_source(timer));
    return fila;
}

// Le o recorde salvo. Se o arquivo ainda nao existe, o recorde continua 0.
void carregar_recorde(void) {
    FILE* arquivo = fopen(ARQUIVO_RECORDE, "r");
    if (arquivo == NULL) return;
    if (fscanf(arquivo, "%d", &recorde) != 1 || recorde < 0 || recorde > PONTOS_MAXIMOS) recorde = 0;
    fclose(arquivo);
}

// Se os pontos passaram o recorde, guarda no arquivo. Devolve true quando e um recorde novo.
bool atualizar_recorde(int pontos) {
    if (pontos <= recorde) return false;
    recorde = pontos;
    FILE* arquivo = fopen(ARQUIVO_RECORDE, "w");
    if (arquivo == NULL) {
        printf("Aviso: nao consegui salvar o recorde.\n");
    }
    else {
        fprintf(arquivo, "%d\n", recorde);
        fclose(arquivo);
    }
    return true;
}

// true se o jogador clicou com o botao esquerdo do mouse
bool clicou(ALLEGRO_EVENT evento) {
    return evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && evento.mouse.button == 1;
}

// Espera o proximo evento. Quando o mouse anda, chegam varios eventos seguidos:
// fica so com o ultimo, para o menu e a pergunta nao desenharem a tela varias vezes a toa.
void esperar_evento(ALLEGRO_EVENT_QUEUE* fila, ALLEGRO_EVENT* evento) {
    ALLEGRO_EVENT proximo;
    al_wait_for_event(fila, evento);
    while (evento->type == ALLEGRO_EVENT_MOUSE_AXES &&
        al_peek_next_event(fila, &proximo) && proximo.type == ALLEGRO_EVENT_MOUSE_AXES) {
        al_get_next_event(fila, evento);
    }
}

// Nas fases, o clique faz o mesmo que o ENTER (comecar, continuar e tentar de novo).
void clique_vira_enter(ALLEGRO_EVENT* evento) {
    if (clicou(*evento)) {
        evento->type = ALLEGRO_EVENT_KEY_DOWN;
        evento->keyboard.keycode = ALLEGRO_KEY_ENTER;
    }
}

// Pontos da fase: comeca com 30 e perde alguns pontos por erro.
// Nunca fica menor que 10.
int calcular_pontos(int erros, int perdaPorErro) {
    int pontos = 30 - perdaPorErro * erros;
    if (pontos < 10) pontos = 10;
    return pontos;
}

// Lousa verde com moldura de madeira, po de giz e a canaleta com dois gizes.
void desenhar_lousa(float x1, float y1, float x2, float y2) {
    al_draw_filled_rectangle(x1 - 28, y1 - 28, x2 + 28, y2 + 40, al_map_rgb(150, 100, 60));   // moldura
    al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(38, 82, 64));                         // quadro

    // po de giz: pontinhos espalhados, sempre nos mesmos lugares (a conta imita um sorteio)
    int largura = x2 - x1 - 4, altura = y2 - y1 - 4;
    for (int i = 0; i < 900; i++) {
        float x = x1 + (i * 7919) % largura;
        float y = y1 + (i * 3571 + i * i * 13) % altura;
        al_draw_filled_rectangle(x, y, x + 2, y + 2, al_map_rgba(40, 45, 42, 60));
    }

    al_draw_filled_rectangle(x1, y2, x2, y2 + 18, al_map_rgb(110, 72, 42));                    // canaleta
    al_draw_filled_rectangle(x2 - 332, y2 - 9, x2 - 272, y2 + 1, GIZ_BRANCO);
    al_draw_filled_rectangle(x2 - 257, y2 - 9, x2 - 217, y2 + 1, GIZ_AMARELO);
}

// Um X desenhado a giz, com o centro em (x, y).
void desenhar_xis(float x, float y, float tamanho, ALLEGRO_COLOR cor) {
    al_draw_line(x - tamanho, y - tamanho, x + tamanho, y + tamanho, cor, 4);
    al_draw_line(x + tamanho, y - tamanho, x - tamanho, y + tamanho, cor, 4);
}

// Desenho da jogada no canto da lousa, um para cada esporte.
void desenhar_jogada(char* esporte) {
    ALLEGRO_COLOR g = GIZ_BRANCO, am = GIZ_AMARELO;

    if (strcmp(esporte, "HANDEBOL") == 0) {         // arremesso passando pela area ate o gol
        float curva[8] = { 880, 470, 920, 380, 1010, 420, 1052, 538 };
        al_draw_arc(1150, 620, 150, ALLEGRO_PI, ALLEGRO_PI / 2, g, 3);
        desenhar_xis(872, 492, 13, am);
        al_draw_circle(1070, 560, 16, am, 4);
        al_draw_spline(curva, g, 3);
        al_draw_filled_triangle(1058, 552, 1040, 530, 1062, 525, g);
    }
    else if (strcmp(esporte, "ATLETISMO") == 0) {   // pista com 3 barreiras e os saltos
        al_draw_line(850, 620, 1145, 620, g, 3);
        al_draw_line(850, 645, 1145, 645, g, 2);
        for (int x = 915; x <= 1075; x += 80) {
            float salto[8] = { x - 40, 612, x - 18, 535, x + 18, 535, x + 40, 612 };
            al_draw_line(x - 14, 620, x - 14, 585, am, 4);
            al_draw_line(x + 14, 620, x + 14, 585, am, 4);
            al_draw_line(x - 18, 585, x + 18, 585, am, 5);
            al_draw_spline(salto, g, 2);
        }
    }
    else {                                          // basquete: arco do arremesso ate a cesta
        float curva[8] = { 885, 580, 940, 410, 1050, 420, 1082, 515 };
        al_draw_line(1120, 470, 1120, 555, g, 5);
        al_draw_ellipse(1085, 535, 30, 8, am, 4);
        for (int i = 0; i <= 3; i++) al_draw_line(1060 + i * 17, 540, 1068 + i * 12, 575, g, 2);
        desenhar_xis(872, 602, 13, am);
        al_draw_spline(curva, g, 3);
        al_draw_filled_triangle(1085, 528, 1068, 505, 1092, 501, g);
        al_draw_line(850, 645, 1145, 645, g, 3);
    }
}

// Tela de regras na lousa: titulo, 5 regras numeradas, a meta, uma imagem e o desenho da jogada.
void tela_regras(char* titulo, char* regras[5], char* meta, ALLEGRO_BITMAP* imagem, float largura, float altura) {
    al_clear_to_color(al_map_rgb(150, 100, 60));
    desenhar_lousa(28, 28, 1172, 690);

    al_draw_text(fonteGizTitulo, GIZ_AMARELO, 80, 55, 0, titulo);
    al_draw_line(82, 142, 162 + al_get_text_width(fonteGizTitulo, titulo), 136, GIZ_AMARELO, 4);

    for (int i = 0; i < 5; i++) {
        float y = 185 + i * 62;
        al_draw_circle(100, y + 20, 19, GIZ_BRANCO, 2);
        al_draw_textf(fonteGiz, GIZ_BRANCO, 100, y, ALLEGRO_ALIGN_CENTER, "%d", i + 1);
        // o + 3 pula o "1. " do comeco do texto, porque o numero ja esta no circulo
        al_draw_text(fonteGiz, GIZ_BRANCO, 135, y, 0, regras[i] + 3);
    }

    al_draw_text(fonteGiz, GIZ_AMARELO, 80, 515, 0, "META:");
    al_draw_text(fonteGiz, GIZ_BRANCO, 180, 515, 0, meta);

    desenhar(imagem, 1020 - largura / 2, 140 - altura / 2, largura, altura, 0);
    desenhar_jogada(titulo);

    al_draw_text(fonteGiz, GIZ_BRANCO, 450, 625, ALLEGRO_ALIGN_CENTER, "ENTER ou clique: comecar   |   ESC: menu");
}

// Tela de fim da fase na lousa: se venceu escreve em amarelo, se perdeu em vermelho.
void tela_fim(bool venceu, char* linha1, char* linha2) {
    ALLEGRO_COLOR corTitulo = venceu ? GIZ_AMARELO : GIZ_VERMELHO;

    al_clear_to_color(al_map_rgb(150, 100, 60));
    desenhar_lousa(28, 28, 1172, 690);

    escrever(fonteGizTitulo, corTitulo, 60, venceu ? "PARABENS!" : "GAME OVER");
    al_draw_line(380, 150, 820, 144, corTitulo, 4);
    escrever(fonteGiz, GIZ_BRANCO, 190, linha1);
    escrever(fonteGiz, GIZ_BRANCO, 245, linha2);

    desenhar(venceu ? ricoPulando : ricoParado, LARGURA / 2 - 90, 320, 180, 270, 0);

    // desenhos nos cantos: bolinhas quando vence, X quando perde
    for (int i = 0; i < 2; i++) {
        float x = 220 + i * 760;
        if (venceu) {
            al_draw_circle(x, 420, 34, GIZ_AMARELO, 4);
            al_draw_circle(x + 50 - i * 100, 500, 18, GIZ_BRANCO, 3);
        }
        else {
            desenhar_xis(x, 420, 30, GIZ_VERMELHO);
            desenhar_xis(x + 50 - i * 100, 500, 15, GIZ_BRANCO);
        }
    }

    if (venceu) escrever(fonteGiz, GIZ_BRANCO, 625, "ENTER ou clique: pergunta final   |   ESC: menu");
    else escrever(fonteGiz, GIZ_BRANCO, 625, "ENTER ou clique: tentar de novo   |   ESC: menu");
}

// Lousa pequena de aviso no meio da tela (infracao do handebol e andada do basquete).
void caixa_aviso(char* titulo, char* linha1, char* linha2) {
    // escurece o jogo atras para o aviso chamar atencao
    al_draw_filled_rectangle(0, 0, LARGURA, ALTURA, al_map_rgba(0, 0, 0, 130));

    desenhar_lousa(240, 190, 960, 540);
    escrever(fonteGizTitulo, GIZ_VERMELHO, 205, titulo);
    al_draw_line(390, 292, 810, 287, GIZ_VERMELHO, 4);
    escrever(fonteGiz, GIZ_BRANCO, 325, linha1);
    escrever(fonteGiz, GIZ_AMARELO, 380, linha2);
    al_draw_ellipse(600, 473, 185, 30, GIZ_BRANCO, 2);
    escrever(fonteGiz, GIZ_BRANCO, 454, "ENTER ou clique");
}

// Barra de forca (ou de velocidade) de 0 a 100: largura = porcento * 2 (0 a 200 pixels).
// A cor vai de verde (fraco) para vermelho (forte).
void desenhar_barra(float porcento) {
    al_draw_filled_rectangle(30, 30, 30 + porcento * 2, 55, al_map_rgb(porcento * 2.5, 200 - porcento * 1.5, 40));
    al_draw_rectangle(30, 30, 230, 55, al_map_rgb(0, 0, 0), 2);
}

// Legenda dos controles no rodape: faixa escura com as teclas em caixinhas brancas.
// teclas[i] vai dentro da caixinha e acoes[i] e escrito do lado dela.
// Se acoes[i] for NULL, a tecla fica junto da proxima (ex.: [A][D] andar).
void desenhar_legenda(int quantidade, char* teclas[], char* acoes[]) {
    float larguras[10];   // largura de cada caixinha (no maximo 10 teclas)
    float total = 0;      // largura da legenda inteira, para centralizar

    for (int i = 0; i < quantidade; i++) {
        larguras[i] = al_get_text_width(fonte, teclas[i]) + 20;
        if (larguras[i] < 36) larguras[i] = 36;
        total = total + larguras[i];
        if (acoes[i] != NULL) total = total + 8 + al_get_text_width(fonte, acoes[i]);
        if (i < quantidade - 1) total = total + (acoes[i] != NULL ? 34 : 6);
    }

    float x = (LARGURA - total) / 2;
    float y = ALTURA - 52;   // topo das caixinhas

    // faixa escura meio transparente (o ultimo numero e a transparencia)
    al_draw_filled_rectangle(x - 20, y - 10, x + total + 20, y + 46, al_map_rgba(0, 0, 0, 170));

    for (int i = 0; i < quantidade; i++) {
        al_draw_filled_rectangle(x, y, x + larguras[i], y + 36, al_map_rgb(255, 255, 255));
        al_draw_text(fonte, al_map_rgb(20, 20, 20), x + larguras[i] / 2, y + 5, ALLEGRO_ALIGN_CENTER, teclas[i]);
        x = x + larguras[i];

        if (acoes[i] != NULL) {
            al_draw_text(fonte, al_map_rgb(255, 255, 255), x + 8, y + 5, 0, acoes[i]);
            x = x + 8 + al_get_text_width(fonte, acoes[i]) + 34;   // espaco ate o proximo comando
        }
        else x = x + 6;
    }
}

// Fisica do pulo do Rico (handebol e basquete).
// A cada frame: posicao += velocidade  e  velocidade += gravidade
// (e a ideia de derivada: a velocidade e a variacao da posicao,
//  a gravidade e a variacao da velocidade -> o pulo vira uma parabola)
// Uso ponteiros para a funcao conseguir mudar as variaveis da fase.
void atualizar_pulo(float* y, float* velY, bool* noChao) {
    *y = *y + *velY;
    *velY = *velY + 0.8;
    if (*y >= CHAO - RICO_A) {   // encostou no chao
        *y = CHAO - RICO_A;
        *velY = 0;
        *noChao = true;
    }
}

// Desenha o Rico com a imagem certa: pulando, andando ou parado.
// Olhando para a esquerda (direcao = -1), a imagem e espelhada.
void desenhar_rico(float x, float y, int direcao, bool noChao, int tempoAndando) {
    ALLEGRO_BITMAP* imagem = ricoParado;
    if (!noChao) imagem = ricoPulando;
    else if (tempoAndando > 0) imagem = ricoAndando;
    desenhar(imagem, x, y, RICO_L, RICO_A, direcao == -1 ? ALLEGRO_FLIP_HORIZONTAL : 0);
}

// Desenha a bola na mao do Rico. No pulo, a bola fica no punho levantado.
// Quicando: tempoDrible vai de 20 ate 0. De 20 a 11 a bola desce, de 10 a 0 a bola sobe.
void desenhar_bola_na_mao(float x, float y, int direcao, bool noChao, int tempoDrible) {
    float maoX = x + (direcao == 1 ? 102 : -3);
    float maoY = y + (noChao ? 100 : 20);
    if (tempoDrible > 0) {
        float distanciaChao = (CHAO - BOLA_T) - maoY;
        int passo = (tempoDrible > 10) ? 20 - tempoDrible : tempoDrible;
        maoY = maoY + distanciaChao * passo / 10.0;
    }
    desenhar(bola, maoX, maoY, BOLA_T, BOLA_T, 0);
}

// Ve se o jogador quer sair da fase.
// Devolve VOLTAR_MENU (ESC ou BACKSPACE), FECHAR_JOGO (X da janela) ou -1 (continua jogando).
int verificar_saida(ALLEGRO_EVENT evento) {
    if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) return FECHAR_JOGO;
    if (evento.type == ALLEGRO_EVENT_KEY_DOWN &&
        (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE || evento.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)) return VOLTAR_MENU;
    return -1;
}

// Teclas do Rico no handebol e no basquete: A/D andar, W pular, S quicar.
// Devolve true se ele tentou dar mais passos do que pode com a bola na mao.
// Uso ponteiros para a funcao conseguir mudar as variaveis da fase.
bool mover_rico(int tecla, int maxPassos, bool comBola, float* x, float* velY, int* direcao,
    bool* noChao, bool* jaMexeuNoAr, int* tempoAndando, int* passos, int* tempoDrible) {
    if (tecla == ALLEGRO_KEY_A || tecla == ALLEGRO_KEY_D) {
        *direcao = (tecla == ALLEGRO_KEY_D) ? 1 : -1;
        if (!*noChao) {
            // no ar: nao conta passo, mas so pode mexer 1 vez
            if (!*jaMexeuNoAr) {
                *x = *x + 60 * *direcao;
                *jaMexeuNoAr = true;
            }
        }
        else if (comBola && *passos >= maxPassos) return true;   // passos demais!
        else {
            *x = *x + 60 * *direcao;   // direcao = -1 faz andar para tras
            *tempoAndando = 8;
            if (comBola) *passos = *passos + 1;
        }
    }
    // W = pular (velocidade negativa = subir, porque o Y cresce para baixo)
    if (tecla == ALLEGRO_KEY_W && *noChao) {
        *velY = -16;
        *noChao = false;
        *jaMexeuNoAr = false;
    }
    // S = quicar a bola: zera os passos
    if (tecla == ALLEGRO_KEY_S && comBola && *noChao && *tempoDrible == 0) {
        *tempoDrible = 20;
        *passos = 0;
    }
    return false;
}
