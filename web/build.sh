#!/bin/sh
# RVIP web build: TSL console frontend (console.c) on the port/ curses subset.
# Needs emcc on PATH (source emsdk_env.sh). Output: web/dist/
set -e
cd "$(dirname "$0")/.."
SRCS=$(sed -n 's/^\t\([a-z0-9_]*\.c\) \\$/\1/p' build_console.sh)
mkdir -p web/dist
emcc -O2 -fcommon -std=gnu99 -DTSL_CONSOLE -DRVIP_AUTO_MORE -Iport \
  $SRCS port/wcurses.c \
  -sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 -sSTACK_SIZE=1048576 \
  -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB \
  -sEXPORTED_FUNCTIONS=_main,_web_key \
  -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,HEAPU8,addRunDependency,removeRunDependency,UTF8ToString \
  -sFORCE_FILESYSTEM -lidbfs.js -sENVIRONMENT=web \
  -o web/dist/slimy.js "$@"
cp web/index.html web/dist/index.html
