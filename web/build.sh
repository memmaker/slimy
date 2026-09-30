#!/bin/sh
# RVIP web build: TSL console frontend (console.c) on the port/ curses subset.
# Needs emcc on PATH (source emsdk_env.sh). Output: web/dist/
set -e
cd "$(dirname "$0")/.."
SRCS=$(sed -n 's/^\t\([a-z0-9_]*\.c\) \\$/\1/p' build_console.sh)
rm -rf web/dist && mkdir -p web/dist
emcc -O2 -fcommon -std=gnu99 -DTSL_CONSOLE -DRVIP_AUTO_MORE -Iport \
  $SRCS port/wcurses.c \
  -sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 -sSTACK_SIZE=1048576 \
  -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB \
  -sEXPORTED_FUNCTIONS=_main,_web_key -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,ENV,HEAPU8,addRunDependency,removeRunDependency,UTF8ToString \
  -sFORCE_FILESYSTEM -lidbfs.js -sENVIRONMENT=web \
  -o web/dist/slimy-core.js "$@"
cp web/index.html web/slimy.js web/dist/
cp tileset.png tiledim.png tilerev.png web/dist/
cp web/tslgo/sprites.png web/dist/tslgo-sprites.png   # tsl-go set, original size
cp -r web/tslgo/music web/dist/music   # tsl-go recorded tracks, one per level (Music option)
python3 web/mksounds.py web/dist/sound
python3 web/make-help.py web/dist/help.html
