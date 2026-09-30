# The Slimy Lichmummy — handover

## RVIP progress
- Stage done: 1 (get + build). Next: stage 2 (explore + stairs + no `--More--`).
- Folder: `slimy-cloud` (repo memmaker/slimy-cloud, private). Base: upstream
  gitlab `master` (only branch), pristine tree = commit `698f45f` (archive.org ArchiveRL.7z).
- Case: R-ish/O. TSL already has a clean UI layer (`ui.h`: `map_*`, `st_*`,
  `mb_*`, `scr_*`) with two backends; chose the **console path under a small
  curses shim**, not an own `allui.c` replacement: `console.c` is 480 lines and
  maps 1:1 onto four curses windows, so each window is already a pane; `allui.c`
  is Allegro-specific (1800 lines). Tiles in stage 4 come from `tiles.c` +
  `tileset.png` via the gent index (`map_put(y, x, gent, attr)`), which
  `console.c` already receives.
- Frontend files: `port/curses.h` + `port/wcurses.c` (curses subset; game
  sources untouched). `wrefresh` routes by window identity: `board_win` → map
  (0), `status_win` → status (1), `message_bar` → messages (2), `stdscr` →
  screen/pop-up (3); sends trimmed HTML lines (colours/reverse from C) through
  `Module.rvipPane(pane, html, rows)`. Keys: `_web_key(k)` queue, `getch`
  loops `emscripten_sleep(10)`; arrows as curses codes 258-261.
- Page: `web/index.html` (stage-1 only, no rvip-wm; exposes `window.slimyShadow`
  text per pane for Playwright).
- Build: `source ~/emsdk/emsdk_env.sh; sh web/build.sh` → `web/dist/`
  (emcc 6.0.10; sources from `build_console.sh`; `-O2 -fcommon -std=gnu99
  -DTSL_CONSOLE -Iport -sASYNCIFY -sASYNCIFY_STACK_SIZE=65536
  -sSTACK_SIZE=1048576 -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB
  -sEXPORTED_FUNCTIONS=_main,_web_key -sFORCE_FILESYSTEM -lidbfs.js`). No `-w`;
  no signature-mismatch warnings (only harmless -Wabsolute-value etc.).
- Tested: headless Chromium (Playwright 1.56.1 at `/opt/node22/lib/node_modules`,
  `NODE_PATH` it): game starts straight into the dungeon ("Hello web_user,
  welcome to TSL!"), moves work, no page errors. ASan+UBSan native build
  (gcc 13, real ncurses, `build_console.sh` sources) driven by pty random keys
  twice ~2000 keys incl. one death: no reports.
- Quirks: no name prompt (uses `$HOME`/user name: web_user); map viewport is
  fixed 40x20 (`board_size_*` in `init_ui`/`set_glyph_mode`) — stage 5 should
  size it to the window; morgue `morgue.txt` written to cwd (`MORGUE_NAME`);
  save path from `$HOME` (`stuff.c`) — IDBFS in 5.10; the game is monochrome
  in console mode except a few `COLOR_PAIR`s in `ui.c`; walls are reverse video;
  `debug.c` calls `getch`/`printw` directly (covered by the shim).
- Open: license is non-free ("contact the author first" for ports) — repo
  private, publishing is the user's call. Visual check in the Mac pane pending (5.17).
