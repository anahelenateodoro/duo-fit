#include "rico_sprites.h"

ALLEGRO_BITMAP *carregar_atlas_rico(void) {
    return al_load_bitmap("assets/rico_sprites.png");
}

ALLEGRO_BITMAP *recortar_rico(ALLEGRO_BITMAP *atlas, int pose) {
    if (!atlas || pose < RICO_PARADO || pose > RICO_CORRIDA) return NULL;
    if (pose == RICO_CORRIDA)
        return al_create_sub_bitmap(atlas, 0, 384, 543, 181);
    return al_create_sub_bitmap(atlas, pose * 256, 0, 256, 384);
}
