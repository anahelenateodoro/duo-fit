#include "ana_sprites.h"

ALLEGRO_BITMAP *carregar_atlas_ana(void) {
    return al_load_bitmap("assets/ana_sprites.png");
}

ALLEGRO_BITMAP *recortar_ana(ALLEGRO_BITMAP *atlas, int pose) {
    if (!atlas || pose < ANA_PARADA || pose > ANA_PULANDO) return NULL;
    return al_create_sub_bitmap(atlas, (pose % 3) * 256,
        (pose / 3) * 384, 256, 384);
}
