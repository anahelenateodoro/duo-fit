#include "navegacao.h"

bool pediu_voltar(const ALLEGRO_EVENT *evento) {
    if (evento->type == ALLEGRO_EVENT_KEY_DOWN)
        return evento->keyboard.keycode == ALLEGRO_KEY_ESCAPE ||
            evento->keyboard.keycode == ALLEGRO_KEY_BACKSPACE;
    return evento->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        evento->mouse.button == 1 && evento->mouse.x >= 970 &&
        evento->mouse.x <= 1180 && evento->mouse.y >= 12 && evento->mouse.y <= 52;
}

void desenhar_voltar(ALLEGRO_FONT *fonte) {
    al_draw_filled_rounded_rectangle(970, 12, 1180, 52, 6, 6, al_map_rgb(20, 45, 100));
    al_draw_text(fonte, al_map_rgb(255, 255, 255), 1075, 17,
        ALLEGRO_ALIGN_CENTER, "Voltar ao menu");
}
