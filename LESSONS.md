# Lessons for RVIP.md (cloud run; merge on the Mac)

- 5.5: a game with its own thin UI layer over a few curses windows (TSL `console.c`: board/status/message windows + stdscr) needs no own frontend: a ~250-line curses subset routing `wrefresh` by window identity gives one pane per window (TSL).
- 5.5: a minimal `port/curses.h` must include `<stdio.h>` — frontends call `printf` relying on real curses.h pulling it in (TSL).
- 5.2: emsdk `latest` in the cloud is emcc 6.0.10; `source emsdk_env.sh` per shell (TSL).
- 5.16: Playwright is preinstalled globally in the cloud (`/opt/node22/lib/node_modules`, 1.56.1, matches `/opt/pw-browsers/chromium-1194`): `NODE_PATH=/opt/node22/lib/node_modules`, no npm install needed (TSL).
