# The Slimy Lichmummy — handover

## RVIP progress
- Stage done: 2 (explore + stairs + no `--More--`). Next: stage 3 (enter menu + inventory).
- Stage 2: explore key `x` (`action_explore`, bound in `common_keys()` so both
  keymaps have it; config name `explore`; help page 1 "auto-explore" + page 2
  text). Code: `explore.c`/`explore.h` (added to `build_console.sh` and
  `build_gui.sh`). Hook: `player_control()` (player.c) asks `explore_step()`
  for a move action before the key read while `explore_mode` is set; one step
  per game turn; painted by `draw_level()` + `web_pause(40)`; a pending key
  (`web_poll_key()` in `port/wcurses.c`) stops it. Known grid:
  `level->memory[y][x]` (gent; `gent_blank` = unknown), walkability from
  `tile_info[map]` only for remembered cells; frontier = remembered passable
  cell next to unknown, until stood on; seen items are targets. Stops: any new
  message (`msg_counter`, bumped in `queue_msg()`; re-baselined after a door
  step), creature in view ("In view: a ratman."), new item in view ("You see
  something new."), a key, a step that did not move. Avoids remembered traps,
  water, lava, force fields and creatures in view (paths through them only if
  nothing else is reachable, which bumps/attacks and stops). Locked doors:
  stop "The door is locked.", skipped afterwards (never uses keys).
  Stairs: TSL stairs are branch links, not up/down; depth = link distance
  from `LEVEL_START` (`ex_depth()`), `<` = stairs to a shallower level, `>` =
  same or deeper, `c` (default keymap) = any. `<`/`>` on matching stairs take
  them, else walk to the nearest known matching staircase and stop on it.
  `--More--`: `-DRVIP_AUTO_MORE` (web build only) in `_msgflush_internal()`
  never waits; overflow shows the last chunk, full text in history `P`.
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
- Open (stage 2): auto_more is compile-time, not an option (TSL options menu
  untouched); overflowing messages show only their last chunk (history `P`
  has all). Per-turn "You are bleeding!" stops explore each step (by RVIP
  rule). Explore may step over stairs/items (no tie-break away from features;
  the arrival message stops it, press again). Locked-door check peeks at the
  true tile of a remembered door. No stop for "no own light" (TSL has torches;
  not checked). Explore not tested deep; tested on levels 1-2 with a local
  immortal test build (not committed).
- Open: license is non-free ("contact the author first" for ports) — repo
  private, publishing is the user's call. Visual check in the Mac pane pending (5.17).
