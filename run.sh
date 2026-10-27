#!/bin/sh
set -eu
cd "$(dirname "$0")"
gcc -Wall -Wextra main.c menu.c handebol.c atletismo.c basquete.c ana_sprites.c rico_sprites.c navegacao.c -o programa $(pkg-config --cflags --libs \
  allegro-5 allegro_main-5 allegro_font-5 allegro_ttf-5 \
  allegro_image-5 allegro_primitives-5)
exec ./programa "$@"
