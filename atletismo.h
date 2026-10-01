#ifndef DUOFIT_ATLETISMO_H
#define DUOFIT_ATLETISMO_H

// Sprint 1: valores em pixels e segundos, separados do handebol.
#define CORRIDA_INICIO 150.0f
#define CORRIDA_CHEGADA 4150.0f
#define CORRIDA_MUNDO 4500.0f
#define CORRIDA_VELOCIDADE_MAX 520.0f
#define CORRIDA_ACELERACAO 260.0f
#define CORRIDA_DESACELERACAO 340.0f
#define CORRIDA_TEMPO_FRAME 0.12f

#define CORRIDA_TROCA_INICIO 1950.0f
#define CORRIDA_TROCA_FIM 2350.0f
#define CORRIDA_TROCA_X 2150.0f
#define CORRIDA_BARREIRAS 6
static const float barreirasX[CORRIDA_BARREIRAS] = {700, 1250, 1750, 2700, 3250, 3800};

typedef struct {
    float x, velocidade, camera, tempo, animacao;
    int frameCorrida, atleta, batidas;
    float salto, velocidadeSalto, aviso;
    bool derrubadas[CORRIDA_BARREIRAS];
    bool teclaD, teclaDireita, terminou;
} Corrida;

static void iniciar_corrida(Corrida *c) {
    *c = (Corrida){0};
    c->x = CORRIDA_INICIO;
}

// Salto so pode comecar no chao; segurar a tecla nao repete o salto.
static void saltar_corrida(Corrida *c) {
    if (!c->terminou && c->salto == 0) c->velocidadeSalto = 650;
}

static void passar_bastao(Corrida *c) {
    if (!c->terminou && c->atleta == 0 && c->salto == 0 &&
        c->x >= CORRIDA_TROCA_INICIO && c->x <= CORRIDA_TROCA_FIM) {
        c->atleta = 1;
        c->x = CORRIDA_TROCA_X;
        c->animacao = 0;
        c->frameCorrida = 0;
    }
}

static void atualizar_corrida(Corrida *c, float dt) {
    if (c->terminou) return;
    float xAnterior = c->x;
    if (c->aviso > 0) c->aviso -= dt;
    c->salto += c->velocidadeSalto * dt;
    if (c->salto > 0 || c->velocidadeSalto > 0)
        c->velocidadeSalto -= 1600 * dt;
    if (c->salto < 0) { c->salto = 0; c->velocidadeSalto = 0; }
    // Dois controles independentes: soltar um nao cancela o outro.
    float anterior = c->velocidade;
    if (c->teclaD || c->teclaDireita) {
        c->velocidade += CORRIDA_ACELERACAO * dt;
        if (c->velocidade > CORRIDA_VELOCIDADE_MAX)
            c->velocidade = CORRIDA_VELOCIDADE_MAX;
    } else {
        c->velocidade -= CORRIDA_DESACELERACAO * dt;
        if (c->velocidade < 0) c->velocidade = 0;
    }
    float deslocamento = (anterior + c->velocidade) * 0.5f * dt;
    if (deslocamento > 0) {
        float restante = CORRIDA_CHEGADA - c->x;
        if (deslocamento >= restante) {
            c->tempo += dt * restante / deslocamento;
            c->x = CORRIDA_CHEGADA;
            c->velocidade = 0;
            c->terminou = true;
        } else {
            c->x += deslocamento;
            c->tempo += dt;
        }
        // O timer troca a pose continuamente, mesmo sem novos eventos de tecla.
        if (c->velocidade > 0) {
            c->animacao += dt;
            while (c->animacao >= CORRIDA_TEMPO_FRAME) {
                c->animacao -= CORRIDA_TEMPO_FRAME;
                c->frameCorrida = 1 - c->frameCorrida;
            }
        }
    } else if (c->x > CORRIDA_INICIO) {
        c->tempo += dt;
    }
    // Ana espera a troca no fim da zona: o percurso exige os dois atletas.
    if (c->atleta == 0 && c->x >= CORRIDA_TROCA_FIM) {
        c->x = CORRIDA_TROCA_FIM;
        c->velocidade = 0;
    }
    for (int i = 0; i < CORRIDA_BARREIRAS; i++) {
        // Teste do intervalo percorrido evita atravessar a barreira entre frames.
        if (!c->derrubadas[i] && xAnterior <= barreirasX[i] + 22 &&
            c->x >= barreirasX[i] - 22 && c->salto < 85) {
            c->derrubadas[i] = true;
            c->batidas++;
            c->velocidade *= 0.4f;
            c->aviso = 1.5f;
        }
    }
    if (c->velocidade == 0) {
        c->animacao = 0;
        c->frameCorrida = 0;
    }
    // Camera segue Ana depois do primeiro trecho e para no fim do mundo.
    c->camera = c->x - 350;
    if (c->camera < 0) c->camera = 0;
    if (c->camera > CORRIDA_MUNDO - LARGURA)
        c->camera = CORRIDA_MUNDO - LARGURA;
}

static void desenhar_corrida(const Corrida *c, ALLEGRO_FONT *fonte,
    ALLEGRO_FONT *grande, ALLEGRO_BITMAP *paradaEsq,
    ALLEGRO_BITMAP *andando, ALLEGRO_BITMAP *andando2,
    ALLEGRO_BITMAP *anaSalto, ALLEGRO_BITMAP *ricoParado,
    ALLEGRO_BITMAP *ricoAndando, ALLEGRO_BITMAP *ricoSalto) {
    ALLEGRO_COLOR branco = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR escuro = al_map_rgb(20, 45, 70);
    al_clear_to_color(al_map_rgb(185, 225, 245));
    al_draw_filled_circle(1050, 170, 45, al_map_rgb(255, 220, 100));
    // Arquibancada com deslocamento menor: profundidade no cenario.
    for (int i = 0; i < 20; i++) {
        float x = i * 280 - c->camera * 0.25f;
        al_draw_filled_rectangle(x, 330, x + 240, 440, al_map_rgb(90, 130, 150));
        for (int j = 0; j < 3; j++)
            al_draw_line(x, 355 + j * 30, x + 240, 355 + j * 30, branco, 3);
    }
    al_draw_filled_rectangle(0, 440, LARGURA, ALTURA, al_map_rgb(70, 155, 85));
    al_draw_filled_rectangle(0, 530, LARGURA, 690, al_map_rgb(190, 80, 55));
    for (int i = 0; i <= 4; i++)
        al_draw_line(0, 530 + i * 40, LARGURA, 530 + i * 40, branco, 3);
    for (int i = 0; i <= 100; i += 10) {
        float x = CORRIDA_INICIO + i * 40 - c->camera;
        if (x < -80 || x > LARGURA + 80) continue;
        al_draw_line(x, 535, x, 685, branco, 2);
        al_draw_textf(fonte, branco, x + 5, 650, 0, "%d m", i);
    }
    float chegada = CORRIDA_CHEGADA - c->camera;
    for (int y = 530; y < 690; y += 20)
        for (int j = 0; j < 2; j++)
            al_draw_filled_rectangle(chegada + j * 20, y,
                chegada + (j + 1) * 20, y + 20,
                ((y / 20 + j) % 2) ? branco : escuro);
    if (chegada <= LARGURA)
        al_draw_text(fonte, escuro, chegada, 490, ALLEGRO_ALIGN_CENTER, "CHEGADA");

    float zona = CORRIDA_TROCA_INICIO - c->camera;
    al_draw_filled_rectangle(zona, 590, CORRIDA_TROCA_FIM - c->camera, 630,
        al_map_rgba(245, 190, 40, 255));
    al_draw_text(fonte, escuro, CORRIDA_TROCA_X - c->camera, 390,
        ALLEGRO_ALIGN_CENTER, "ZONA DE TROCA - E");
    // O parceiro fica visivel esperando ou apos terminar sua parte.
    ALLEGRO_BITMAP *esperando = c->atleta == 0 ? ricoParado : paradaEsq;
    float parceiroX = c->atleta == 0 ? CORRIDA_TROCA_X : CORRIDA_TROCA_INICIO;
    al_draw_scaled_bitmap(esperando, 0, 0, al_get_bitmap_width(esperando),
        al_get_bitmap_height(esperando), parceiroX - c->camera - 70,
        380, 140, 210, c->atleta == 0 ? 0 : ALLEGRO_FLIP_HORIZONTAL);
    for (int i = 0; i < CORRIDA_BARREIRAS; i++) {
        float x = barreirasX[i] - c->camera;
        if (x < -60 || x > LARGURA + 60) continue;
        if (c->derrubadas[i]) {
            al_draw_filled_rectangle(x - 35, 622, x + 35, 630, branco);
        } else {
            al_draw_line(x - 25, 630, x - 25, 550, escuro, 5);
            al_draw_line(x + 25, 630, x + 25, 550, escuro, 5);
            al_draw_filled_rectangle(x - 32, 542, x + 32, 553, branco);
            for (int j = 0; j < 4; j++)
                al_draw_filled_rectangle(x - 32 + j * 16, 542,
                    x - 24 + j * 16, 553, al_map_rgb(220, 60, 50));
        }
    }

    ALLEGRO_BITMAP *sprite = paradaEsq;
    int flags = ALLEGRO_FLIP_HORIZONTAL;
    if (c->velocidade > 0) {
        // Alterna diretamente ana_andando1.png e ana_correndo2.png.
        sprite = c->frameCorrida == 0 ? andando : andando2;
        flags = 0;
    }
    if (c->atleta == 1) {
        sprite = c->velocidade > 0 ? ricoAndando : ricoParado;
        flags = 0;
    }
    if (c->salto > 0) {
        sprite = c->atleta == 0 ? anaSalto : ricoSalto;
        flags = 0;
    }
    al_draw_scaled_bitmap(sprite, 0, 0, al_get_bitmap_width(sprite),
        al_get_bitmap_height(sprite), c->x - c->camera - 70,
        420 - c->salto, 140, 210, flags);
    // Bastao acompanha a mao do atleta que esta correndo.
    float maoX = c->x - c->camera + 35;
    al_draw_line(maoX, 520 - c->salto, maoX + 25, 505 - c->salto,
        al_map_rgb(255, 215, 20), 7);
    al_draw_textf(fonte, escuro, 600, 145, ALLEGRO_ALIGN_CENTER,
        "Correndo: %s | Barreiras derrubadas: %d", c->atleta == 0 ? "Ana" : "Rico", c->batidas);
    if (c->atleta == 0 && c->x >= CORRIDA_TROCA_INICIO)
        al_draw_text(fonte, escuro, 600, 180, ALLEGRO_ALIGN_CENTER,
            "Passe o bastao ao Rico: pressione E no chao!");
    if (c->aviso > 0)
        al_draw_text(fonte, escuro, 600, 220, ALLEGRO_ALIGN_CENTER,
            "Barreira derrubada! Salte um pouco antes com ESPACO.");

    al_draw_text(fonte, escuro, LARGURA / 2, 20, ALLEGRO_ALIGN_CENTER,
        "DuoFit - Fase 2: Atletismo");
    float metros = (c->x - CORRIDA_INICIO) / 40;
    al_draw_textf(fonte, escuro, LARGURA / 2, 60, ALLEGRO_ALIGN_CENTER,
        "Percurso: %.1f / 100 m | Velocidade: %.1f m/s | Tempo: %.2f s",
        metros, c->velocidade / 40, c->tempo);
    al_draw_filled_rectangle(250, 110, 950, 126, al_map_rgb(130, 170, 185));
    al_draw_filled_rectangle(250, 110, 250 + 700 * metros / 100, 126,
        al_map_rgb(245, 150, 40));
    al_draw_text(fonte, branco, LARGURA / 2, 710, ALLEGRO_ALIGN_CENTER,
        "D / seta: correr | ESPACO: saltar | E: bastao | R: reiniciar | ESC: menu");
    if (c->terminou) {
        al_draw_filled_rounded_rectangle(260, 200, 940, 385, 16, 16, escuro);
        al_draw_text(grande, branco, 600, 220, ALLEGRO_ALIGN_CENTER, "CHEGOU!");
        al_draw_textf(fonte, branco, 600, 300, ALLEGRO_ALIGN_CENTER,
            "Ana + Rico: %.2f s | Barreiras derrubadas: %d", c->tempo, c->batidas);
        al_draw_text(fonte, branco, 600, 345, ALLEGRO_ALIGN_CENTER,
            "R para correr novamente | ESC para o menu");
    }
}
#endif
