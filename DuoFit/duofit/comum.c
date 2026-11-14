// DuoFit - funcoes usadas por todas as fases

#include "duofit.h"

// variaveis globais (declaradas com extern no duofit.h)
ALLEGRO_FONT* fonte = NULL;
ALLEGRO_FONT* fonteGrande = NULL;
ALLEGRO_FONT* fonteMenu = NULL;

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

// Carrega tudo do jogo. Devolve false se faltar alguma imagem.
bool carregar_recursos(void) {
    fonte = abrir_fonte(24, false);
    fonteGrande = abrir_fonte(56, true);
    fonteMenu = abrir_fonte(32, false);

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
    if (timer != NULL) al_register_event_source(fila, al_get_timer_event_source(timer));
    return fila;
}

// Pontos da fase: comeca com 30 e perde alguns pontos por erro.
// Nunca fica menor que 10.
int calcular_pontos(int erros, int perdaPorErro) {
    int pontos = 30 - perdaPorErro * erros;
    if (pontos < 10) pontos = 10;
    return pontos;
}

// Tela de regras (fundo azul): titulo, 5 regras, a meta e uma imagem.
// A regra 4 e sempre a dica especial, entao fica amarela.
void tela_regras(char* titulo, char* regras[5], char* meta, ALLEGRO_BITMAP* imagem, float largura, float altura) {
    al_clear_to_color(al_map_rgb(20, 45, 100));
    escrever(fonteGrande, al_map_rgb(255, 140, 0), 50, titulo);
    for (int i = 0; i < 5; i++) {
        ALLEGRO_COLOR cor = al_map_rgb(255, 255, 255);
        if (i == 3) cor = al_map_rgb(255, 220, 0);
        escrever(fonte, cor, 170 + i * 50, regras[i]);
    }
    escrever(fonte, al_map_rgb(0, 255, 120), 440, meta);
    desenhar(imagem, LARGURA / 2 - largura / 2, 490, largura, altura, 0);
    escrever(fonte, al_map_rgb(255, 255, 255), ALTURA - 60, "ENTER para comecar  |  ESC/BACKSPACE para o menu");
}

// Tela de fim da fase: verde se venceu, vermelha se nao cumpriu a meta.
void tela_fim(bool venceu, char* linha1, char* linha2) {
    if (venceu) {
        al_clear_to_color(al_map_rgb(20, 100, 60));
        escrever(fonteGrande, al_map_rgb(255, 255, 255), 150, "PARABENS!");
    }
    else {
        al_clear_to_color(al_map_rgb(120, 25, 25));
        escrever(fonteGrande, al_map_rgb(255, 255, 255), 150, "NAO PASSOU DE FASE!");
    }
    escrever(fonte, al_map_rgb(255, 220, 0), 250, linha1);
    escrever(fonte, al_map_rgb(255, 255, 255), 290, linha2);
    desenhar(venceu ? ricoPulando : ricoParado, LARGURA / 2 - 90, 330, 180, 270, 0);
    if (venceu) escrever(fonte, al_map_rgb(255, 255, 255), ALTURA - 60, "ENTER: responder a pergunta final  |  ESC: menu");
    else escrever(fonte, al_map_rgb(255, 255, 255), ALTURA - 60, "ENTER: tentar a fase de novo  |  ESC: menu");
}

// Caixa vermelha de aviso no meio da tela (infracao do handebol e andada do basquete).
void caixa_aviso(char* titulo, char* linha1, char* linha2) {
    al_draw_filled_rectangle(280, 235, 920, 515, al_map_rgb(70, 20, 20));
    al_draw_rectangle(280, 235, 920, 515, al_map_rgb(255, 255, 255), 3);
    escrever(fonte, al_map_rgb(255, 80, 80), 280, titulo);
    escrever(fonte, al_map_rgb(255, 255, 255), 335, linha1);
    escrever(fonte, al_map_rgb(255, 220, 0), 380, linha2);
    escrever(fonte, al_map_rgb(0, 255, 120), 450, "ENTER para continuar");
}

// Barra de forca (ou de velocidade) de 0 a 100: largura = porcento * 2 (0 a 200 pixels).
// A cor vai de verde (fraco) para vermelho (forte).
void desenhar_barra(float porcento) {
    al_draw_filled_rectangle(30, 30, 30 + porcento * 2, 55, al_map_rgb(porcento * 2.5, 200 - porcento * 1.5, 40));
    al_draw_rectangle(30, 30, 230, 55, al_map_rgb(0, 0, 0), 2);
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
