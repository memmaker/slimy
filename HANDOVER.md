# The Slimy Lichmummy — handover

## RVIP progress
- **PUBLISHING BLOCKER (tsl-go assets, 2026-09-30):** the user asked to add
  the tiles and sounds of github.com/c0ze/tsl-go (Go port of TSL, @0c62dcf).
  Licence check: the repo has no licence of its own ("The Go port is offered
  under the same terms" as TSL = not free). Tiles: `assets/tiles/dcss/` are
  Dungeon Crawl Stone Soup tiles, **CC0** (clean); `gen/` + `anim/` (35
  sprites incl. player, ratman, ghoul, doors, stairs, water/lava) are
  "Project art", AI-generated (Google Antigravity / Qwen-Image), **no licence
  stated**. Music (`web/audio/*.mp3`, cozy-tracker scores): project-made, **no
  licence stated**. SFX: tsl-go's Web Audio recipes (its own code, TSL terms).
  Wired anyway (explicit request); before any publishing ask c0ze for
  permission or ship only the CC0 DCSS subset. Credits are in Help and
  `port/publish/README.md`.
- tsl-go sounds + music: tsl-go's effects are Web Audio recipes (its
  `index.html` `sfx`), not samples; `web/mksounds.py` renders them to wav with
  the same parameters (TSLGO table: oscillator glides + RBJ biquad noise,
  8 ms attack / exp decay, normalised) for our events hit, hurt, death,
  pickup, eat, quaff, read, wear (=equip), stairs (=descend), spell (=cast);
  kill, shoot, drop, teleport keep the synthesized ones (tsl-go's swoosh/zap
  are melee swing/wand, not our events; door/step/splash have no hook here).
  Music: new **Music** checkbox in Audio (off by default, stored as
  `audio.music` in web-layout.json); C `web_level(level_index)` from
  `draw_level` (ui.c, on change) -> `Module.rvipLevel` -> loops
  `music/<level>.mp3` (tsl-go's recorded track `<level>-1.mp3` per level,
  vendored in `web/tslgo/music/`, 25 MB; the `-2` tracks and tsl-go's
  adaptive tracker modules not taken). Audio element made only when Music on.
- Tested (Playwright): sound and music off by default, nothing fetched;
  after real clicks: sounds.json + drop/wear/pickup wavs fetched and played
  on those actions; Music -> `music/dungeon.mp3` fetched and playing; both
  kept over reload; no page errors. Not heard by a person yet (Mac pane).
- tsl-go tiles: second set, `Tiles` cycles by name `Tiles` (game's own,
  label "TSL") -> `tsl-go` -> `None`, stored in `/slimy/web-layout.json`.
  Sheet `web/tslgo/sprites.png` (tsl-go's atlas, 512x480, 32 px, 16 per row,
  original size) -> `dist/tslgo-sprites.png`; `web/tslgo/sprites.js` is its
  index, vendored unchanged as the generator's input. `web/mktslgo.py` reads
  it + the gent enum (`gcc -E gent.h`), hand tables DIRECT/STAND, asserts
  every drawn gent is mapped and every name exists, writes
  `port/tslgo_tiles.h` (`tslgo_gent[gent]`, per-level wall/floor families from
  tsl-go's THEMES keyed by LEVEL_*). C (`map_put`, console.c) computes a
  second code `top | under<<8 | dim<<16` (255 = none; variant by level cell)
  -> `web_map_tile2` -> second array of `Module.rvipMap`. JS draws the chosen
  set, dims MAP_DIM/SLEEP cells with a black overlay (no dim sheet in
  tsl-go), no animation (frame 0 of water/lava/player).
- Coverage: 155 drawn gents, 69 with a tsl-go sprite made for them (45%),
  86 same-set stand-ins (11 wall shapes + obstacle/block -> level wall,
  weapons/guns -> staff/bow, food -> ration, spell/arrow bolts -> wisp/arrows,
  ...) = 100% with stand-ins, but **own-art coverage is well under 95%**: many
  items look alike (all food, all guns). Killer PNGs stay on the own set.
- Tested (Playwright): own set and tsl-go render (screenshots), tsl-go
  survives reload, cycle TSL -> tsl-go -> None -> TSL, no page errors.
- Stages done (cloud parts): 7 (publish prep), 8 (shrine prep), 9 (graveyard).
  Next: the Mac session. Nothing is public; publishing at all needs the
  user's decision (licence: contact the author, Ulf Åström, first).
- **Mac session steps:** (1) decide on publishing / contact the author;
  (2) `git pull`, build, visual check in the pane (tiles, windows, help,
  sound, a death + quit); (3) repo per stage 7 only if publishing
  (memmaker/slimy, README upstream link + compare view); (4) apply
  `port/publish/`: `card.html`, `tree.html`, `years.json.txt`, `slimy.png`
  -> `img/slimy.png`, run og.py for slimy (expected block `og.html`), drop the
  page's old `<meta name="description">`, Help "About this version" line;
  (5) shrine: `port/publish/shrine/slimy.html` + `shrine/slimy/`, then the
  Info button, tree ✦ and `#bar h1` link, 375 px check; (6) killers:
  `port/publish/killers/slimy/*.png` -> `killers/slimy/`, `slimy()` in
  make.py (see `port/publish/README.md`); (7) `web/deploy.sh` + roguelikes
  deploy.sh, check live, one real death in the user's browser on the graveyard.
- Stage 9: beacon `g=slimy&ev&name&killer&depth&turns` (+ id/at by
  RvipWM.report). C: `run_report(ev, reason)` in losegame.c, called in
  `check_for_player_death()` right after `game_over = true` (before morgue and
  game-over key waits; reason "quit" = the `Q` path in player.c -> ev=quit,
  else death) and in `win_game()` (wingame.c, ev=win, before `game_over()`;
  win_game now also calls `web_end()` like death). Save-and-quit (S) sends
  nothing. Killer: `set_killer(creature)` before the death call in combat.c
  (melee), missile.c (creature missiles), magic.c (crush, electrocute, frost
  ray) = `name_one` minus a/an/the (`name_only` is "bah" for the wolves);
  other deaths send the reason text minus "were " ("drowned", "killed by an
  explosion"). Reset after each report. depth = link distance from the start
  area + 1 (`explore_depth()`, explore.c; TSL has no numbered depth). EM_JS
  `web_beacon` (port/wcurses.c) -> `Module.rvipBeacon` (web/slimy.js) builds
  the URL-encoded query and sends via `RvipWM.report`. Name: TSL names the
  hero from getlogin/getpwuid (`web_user` in wasm), so the page asks once
  (`window.prompt` at the first run end), keeps it in `/slimy/web-name`
  (blank = no name, not asked again).
- Missing fields: `score` (TSL has no score or high-score list), `lvl` (no
  character levels: facets instead). Killers without art: Argor, Ghrazghaar,
  Sulkor, Cha'ajd, Sir Lognac, Ybznek (uniques with no gent set in unique.c;
  page falls back to text), disguised mimics (item names).
- Killer PNGs: `port/publish/killers.py` -> 44 PNGs, 32 px, from tileset.png
  gent slots with the console.c stand-ins, magenta transparent.
- Tested (Playwright, beacon intercepted with `page.route`): quit (`Q`,`y`)
  -> `ev=quit&name=Quitter&depth=1&turns=1&id&at`; death (scratch build
  health=1) -> `ev=death&killer=gnoblin|ratman&depth=1&turns=…`, name asked
  once; win (scratch build calling `win_game` at turn 3, not committed) ->
  `ev=win&name=Winner&depth=1&turns=3`. No page errors. Outbox 503/resend
  not re-tested (shared rvip-wm code). Real win path (Chapel of Fallen Stars
  win trap, traps.c) not reached by play.
- Stage 8: shrine draft `port/publish/shrine/` (see NOTES.md there);
  happyponyland.net, archive.org, RogueBasin blocked from the cloud; no
  walkthrough found; manual = README.md (+ CHANGES.TXT), unmodified copies.
  Shrine page not rendered/tested at 375 px yet.
- Stage 7: drafts in `port/publish/` (README.md there lists them). Year 2006
  (backloggd, Roguetemple); CHANGES.TXT's first date is 0.3 (2007-09-22):
  cross-check RogueBasin on the Mac. Version line: 0.40 ·
  vitaly-zdanevich/the-slimy-lichmummy @ 6f885be.
- Stage 6: Help = `web/make-help.py` -> `dist/help.html` (self-contained,
  shaped like a Docs GAMES/GUIDES entry; on the Mac move it into
  build-docs.py/guides.py). Content: README.md + help.c pages in own words;
  keys from keymap.c (`common_keys` + `default_keymap`, QWERTY) plus the port's
  x, `<`/`>` walk, Enter menu, inventory letter/Shift/Enter; saving written
  for the web (one save, deleted on load). 49 rows in the full list.
  Sound: `RVIP_SOUND(e)` in main.h (no-op off the web) -> `web_sound`
  (port/wcurses.c) -> `Module.rvipSound` -> `RVIPSound.play`. Events (player
  only): hit, hurt (melee_hit), kill (creature_death), shoot (fire_missile),
  pickup, drop, stairs, eat (after the edible check), quaff, read (scroll +
  book), wear (equip_item), spell (invoke_ability success), teleport, death
  (check_for_player_death). `web/mksounds.py` synthesizes one wav per event
  (asserted against the C). Off by default; sounds.json fetched only when on
  (or at load if saved on). No music; the Music checkbox was removed.
  Sound search (2026-09): TSL upstream has no audio; a third-party Go port
  (github.com/c0ze/tsl-go, tsl.coze.org) has its own music/sfx, licence not
  checked, not used.
- Tested (Playwright): help opens, Esc closes; sound off by default, nothing
  fetched; after a real click: drop/pickup/wear/eat play per action, "can't
  eat that" silent; kept on over reload. Death path in the browser (local
  build with health forced to 1, scratch, not committed): hurt + death
  sounds, `web_end` -> "Game over. Starting a new game…" -> reload, no page
  errors. Open: stairs/spell/teleport/kill/shoot sounds not exercised in a
  test; auto-equip plays wear twice (weapon + ammo); real look/listen in the
  Mac pane still required.
- Stage 5: **ready to deploy** (cloud cannot deploy; `web/dist` built and
  tested headless). Live URL once deployed: https://ruzzoli.de/roguelikes/slimy/
  (needs `web/deploy.sh` from the Mac; publishing needs the user's decision,
  licence). Page = `web/index.html` + `web/slimy.js` (page code), emcc output
  renamed `slimy-core.js` (RvipApp crash tag); loads `../rvip-{wm,app}.js`.
  Windows (rvip-wm): Map (canvas tiles or `<pre>` text), Messages (message
  bar), Status (status pane, also holds TSL's inventory browser); stdscr
  (command menu, help, item menus, game over) = `#pop` via `RvipWM.popup`,
  text size = Messages'. A−/A+ per window kept by the WM; the Map's size is
  its text font, tiles step in whole multiples: scale = fs-15 (16 = 1x ..
  19 = 4x, `fontMax.map` 19). Camera: C sends the hero's board cell
  (`web_map_hero` from `draw_level`, ui.c) with every map flush;
  `RvipWM.center`. TSL itself scrolls its 40x20 view.
  Saves/settings: IDBFS at `RvipApp.dir` (`/slimy`), `ENV.HOME` = it, so
  `TSL-SAVE` and `.tsl_conf` land there; page settings in
  `/slimy/web-layout.json` (wm state, tiles by name 'Tiles'/'None', fonts,
  audio). No localStorage left. Save atomic (`TSL-SAVE.tmp` + rename,
  saveload.c). Save-and-quit (S) and death call `web_end()` (EM_ASYNC_JS:
  sync, reload = next game / restored save); loading the save deletes it
  (TSL rule) and `web_sync()` persists that. Sync every 15 s, on hide, pagehide.
- Stand-ins (separate commit): the 16 empty-slot gents now draw a same-sheet
  sprite (amulet->crown, beetle_shell/fish/meat->carcass, bone_dust->bone,
  caeltzan->necromancer, chickpeas/falafel->bread, cranium->decapitated_head,
  eyeball->floating_brain, lognac->goatman, mandrake_root->mushroom,
  mummy_wrapping->robe, prod->staff, sausage->cheese, ybznek->nameless_horror):
  150/150 drawn gents tiled (100%).
- Tested (Playwright, scratch www with symlinks to dist + rvip js + fonts):
  shared smoke (bar order, File/Audio drop-downs, A+ on Messages only, kept
  over reload, tiles switch, IDB names `/slimy` only), idbtest (tiles None
  survives reload, file in IDBFS, localStorage 0), resize (1280..420 px, every
  divider to both ends, font sizes fixed, windows scroll, no negative sizes),
  Enter menu pop-up, `i`, `x`, map zoom 2x follows the hero, S saves -> reload
  -> "welcome back" -> save removed; no page errors.
- Open (stage 5): no separate Inventory/Visible windows (TSL shows the
  inventory in its status pane; a Visible list needs a C hook); Messages is
  TSL's 2-line message bar, no history log, no `RvipWM.prompt`; one-window
  mode = same windows without title bars, not a scaled full-screen canvas; no
  autosave before descending (TSL saves only as save-and-quit and deletes the
  save on load, so a crash loses the run); death path `web_end` untested in
  the browser (same hook as save); Help loads `help.html` which stage 6 makes;
  audio checkboxes persist but play nothing (stage 6). Real look in the Mac
  pane still required.
- Stage 4: one set = the game's own `tileset.png` (+ `tiledim.png` for
  MAP_DIM/MAP_SLEEP, `tilerev.png` for MAP_REVERSE), 337x833, shipped at
  original size (copied by `web/build.sh` into `web/dist/`). Slot of gent g:
  x = 1 + (g%16)*21, y = 161 + (g/16)*21, 20x20 (allui.c `gent_rect`).
  C decides (rule 7): `map_put()` (console.c, `__EMSCRIPTEN__`) calls
  `web_map_tile(y,x, top | under<<10 | sheet<<20)` — under = `gent_floor +
  location->floor_type` (drawn first, as allui.c), top = gent (floor ->
  under); gents exceed 255, hence 10 bits. `port/wcurses.c` keeps the code
  plus the cell char+attr it was set with; on the board `wrefresh` it sends
  `Module.rvipMap(tiles, cells, h, w)`, a tile only while the cell still
  holds that char+attr (else -1 = text drawn from the cell). Canvas `#mapc`
  is the only canvas; scale 1 (20 px cells, `SCALE` in index.html, integer
  zoom only, `imageSmoothingEnabled=false` + pixelated). Text mode = the old
  `<pre id="map">`. Button "Tiles: on/None" bottom right toggles; stored by
  name ('Tiles'/'None') in localStorage `web-tiles`, read before the sheets
  load; generation counter guards late onload.
- Coverage (Playwright, pixel check of each enum slot): 152 gent ids,
  18 empty slots: gent_blank (intentionally black) and gent_floor (floor
  type 0) are fine; 16 real gaps (amulet, beetle_shell, bone_dust, caeltzan,
  chickpeas, cranium, eyeball, falafel, fish, lognac, mandrake_root, meat,
  mummy_wrapping, prod, sausage, ybznek) -> 136/152 = 89% tiled; the gaps go
  out as the text glyph (C table `no_tile[]` in map_put), so nothing is
  invisible. The original Allegro build draws them as bare floor.
- Tested: tiles show (walls, floor, water, @, ratman, electric snake), 60+
  tile cells, toggle to None hides canvas, None survives reload, back on
  works, no page errors.
- Open (stage 4, fixed in stage 5): coverage now 100% with stand-ins; pref
  moved from localStorage to IDBFS. Tiles only 1x (800
  px map); stage 5 decides zoom. Explore/animation frames (allui anim.c
  effects) not ported; the text shadow `slimyShadow.map` still holds the
  text map.
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
