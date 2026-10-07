#!/bin/sh
gcc mainn.c handebol.c atletismo.c -o programa $(pkg-config --cflags --libs \
  allegro-5 allegro_main-5 allegro_font-5 allegro_ttf-5 \
  allegro_image-5 allegro_primitives-5) && ./programa
