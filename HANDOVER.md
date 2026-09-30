# The Slimy Lichmummy — handover

## RVIP progress
- Stage done: 3 (Enter menu + inventory). Next: stage 4 (tiles).
- Stage 2 re-check (stage-3 agent, final build + local immortal copy): `>`
  walk stops on the stair, second press descends; on the new level `<` walks
  back to the arrival stair (one step per press while "You are bleeding!"
  stops it), stops "Here is a stair leading to the Dungeon.", second press
  climbs. Birth: TSL has no character creation (straight into the dungeon,
  no `--More--`, screen pane empty). No code change needed.
- Stage 3: Enter command menu = `cmdmenu.c`/`cmdmenu.h` (`command_menu()`;
  added to both build scripts). Table `cmd_table[]` grouped as help.c's key
  reference (Actions, Items, Missiles, Abilities, Game); no movement or
  fire-in-direction entries; `<`/`>` walks listed as own rows. Keys by reverse
  lookup in `keymap[action][]` (printable first, Enter skipped) so the
  current keymap shows. Drawn on stdscr (pane 3) at 0,0 with a box sized to
  the longest row/title; scrolls (`^`/`v` marks) when taller than `LINES-2`.
  Arrows/NumPad8/2 move, Enter/NumPad5 choose, a command's own key chooses,
  Esc/Space/./NumPad0 close. Returns the key; `player_control()` (player.c)
  sets `last_key` + `key_to_action()` so stairs direction and every prompt
  work as typed (unbound action: returned action used directly). Enter
  (`'\n'`, was `action_select` = do-what-I-mean) now opens the menu at the
  main prompt; NumPad5 keeps do-what-I-mean (also a menu row).
- Inventory: TSL already had a cursor browser (`browse()` in browser.c, list
  drawn in the status pane via `st_*`) and an item submenu
  (`browser_item_submenu()` in dwiminv.c, formerly Tab). Added in `browse()`
  for `MENU_USE` (`i`): letter = main action (`dwim_item`, via
  `action_select`), Shift+letter = drop, Ctrl+letter = cursor to it
  (description = examine), Enter/Space/NumPad5 = item menu (`action_flip`),
  `+` main, `-` drop, `*` nothing (examine is always shown), `.`/NumPad0
  close. All lists: NumPad8/2 move. Item submenu (`MENU_GENERIC`): NumPad4
  back, NumPad6 choose. Hint row now "<letter> to <verb>, Enter: menu"
  (`menu_item_add_explanation`, menuitem.c), hidden in item prompts
  (`MENU_PICK`). Reopen: `dwim_inventory()` sets `inv_reopen` when an item
  action that took a turn closed the list; `player_control()` reopens it next
  turn unless `can_see_anyone()`.
- Item prompts ("What do you want to equip?"): TSL's `dwim_select()` already
  shows the same browser with a cursor (arrows/NumPad move, Enter/NumPad5
  choose, letters move the cursor as before). No inventory/equipment/floor
  switch: TSL lists equipped items in the same list; floor pickup is its own
  browser (`MENU_PICKUP`). No `@`-tags in TSL.
- Numpad: `web/index.html` sends `Numpad0-9` (by `e.code`, any NumLock) as
  0x1000+digit; `get_keypress()` (console.c, `__EMSCRIPTEN__`) maps them to
  `kt_np0..9`. Before, numpad digits arrived as `1`..`0` = ability shortcuts.
- Tested (Playwright, final build): menu shows/scrolls, Esc closes, arrows +
  Enter run `use item`, `x` from the menu explores, `i` from the menu; `i` then
  Enter = item menu (Put away/Drop/Eat/Label), NumPad2+5 = menu of item b,
  `b` = main action (put ammo away), `B` = drop (How many?), `C` = drop torch,
  `e` prompt with cursor, ArrowDown+Enter chooses. No page errors. Native
  gcc console build compiles (needs `-fcommon`, as before).
- Open (stage 3): the inventory browser is the game's own status-pane view
  (fixed 38 columns), not a floating window sized to content — stage 5 pane
  layout decides; cursor rows show only as reverse video (not in the text
  shadow). Reopen after a turn-taking item action not exercised in a test
  (no consumable in the starting kit). Mouse: none (curses shim).
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
