# Lessons for RVIP.md (cloud run; merge on the Mac)

- 5.5: a game with its own thin UI layer over a few curses windows (TSL `console.c`: board/status/message windows + stdscr) needs no own frontend: a ~250-line curses subset routing `wrefresh` by window identity gives one pane per window (TSL).
- 5.5: a minimal `port/curses.h` must include `<stdio.h>` — frontends call `printf` relying on real curses.h pulling it in (TSL).
- 5.2: emsdk `latest` in the cloud is emcc 6.0.10; `source emsdk_env.sh` per shell (TSL).
- 5.16: Playwright is preinstalled globally in the cloud (`/opt/node22/lib/node_modules`, 1.56.1, matches `/opt/pw-browsers/chromium-1194`): `NODE_PATH=/opt/node22/lib/node_modules`, no npm install needed (TSL).
- 5.6: games whose stairs are branch links, not up/down (TSL `tile_stair0..3` + `level->link[]`): define `<`/`>` by link distance from the start level (BFS over links); a `<` on a down-stair must not fall back to taking it, or `<`/`>` ping-pong between two levels (TSL).
- 5.6: stair walks and explore should path around creatures in view and only go through them when nothing else is reachable (then the bump attack stops the walk); otherwise a monster in a corridor turns every press into "You can not attack yet!" (TSL).
- 5.6: in a Playwright bot, test explore with a local immortal build (health reset each turn, copied tree in scratch, never committed) so walks reach stairs; a mortal bot dies to archers before the level is done (TSL).
- 5.6: a curses shim with a JS key queue gives the "any key stops" check for free: a non-blocking `web_poll_key()` popping the queue, plus `web_pause(ms)` = `emscripten_sleep` for the 40 ms paint (TSL).
