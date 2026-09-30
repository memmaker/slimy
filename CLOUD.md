# The Slimy Lichmummy (TSL) — RVIP cloud run brief

You are a cloud agent porting **The Slimy Lichmummy 0.40** to the web following
the RVIP (Roguelike Variant Import Procedure). This file is your whole context;
the user is not around to answer questions. Web name: `slimy`
(`https://ruzzoli.de/roguelikes/slimy/`).

## What the game is
Roguelike by Ulf Åström (happyponyland.net), v0.40 (2012). Plain C, ~50k lines,
single directory. Two frontends selected at compile time:
- `-DTSL_CONSOLE` (`build_console.sh`): curses, `console.c`.
- `-DTSL_GUI` (`build_gui.sh`): Allegro 5, `allui.c` (tiles + animation).
Drawing goes through `ui.c`/`glyph.c`; `tiles.c`, `tileset.png` (337x833),
`tiledim.png`, `tilerev.png` are the game's **own tileset** (use it, RVIP 5.8).
Fonts: `font.png`, `fontdim.png`, `fontrev.png`, `smallfont.tga` (modified Terminus, OFL).
Config: `tsl_conf_example`, `tsl_conf_dvorak` (keymaps). Help: in-game `?`
(`help.c`), `README.md` = manual. `CHANGES.TXT` history. `web.c` is game code
(spider webs), not a web frontend.

Source: https://gitlab.com/vitaly-zdanevich/the-slimy-lichmummy (a 2025 upload
from archive.org ArchiveRL.7z; 5 commits; add it as remote `upstream`). `origin` = this private repo
(memmaker/slimy-cloud). Commit 1 of the upstream history is the pristine tree.

## License — not free software (read `LICENSE.TXT`)
Unmodified source may be redistributed; "unofficial patches or ports" are
tolerated but the author asks to be contacted first; no selling. So: keep this
repo **private**, publish nothing, and write in `HANDOVER.md` that publishing
needs the user's decision (contact the author). Stage 7/8 public steps are for
the Mac session.

## Setup
```
git clone https://github.com/memmaker/rvip /home/user/rvip                # RVIP.md, web/rvip-{wm,app,sound}.js, tests/
git clone https://github.com/memmaker/roguelikes /home/user/roguelikes    # index, tree, shrine, deploy.sh
git clone https://github.com/memmaker/rogue3.6 /home/user/rogue3.6        # curses shim, windows
git clone https://github.com/memmaker/avanor /home/user/avanor            # recent C/C++ precedent, own-shim
```
Read-only for you: don't commit into those clones. `RVIP_WEB=/home/user/rvip/web`,
`ROGUELIKES=/home/user/roguelikes`. Never copy `rvip-*.js` into this repo.
(A refused clone: note it in `HANDOVER.md`, work from the others.)

## Procedure
Read `/home/user/rvip/RVIP.md`: part 1 (orchestration, checkpoint), part 2 (hard
rules, all of them, incl. presentation rules), part 3, the stage in part 4 you
are on, and the part 5 topics it names, especially **5.5 (curses shims), 5.8
tiles, 5.9 windows, 5.10 saves, 5.16 (Playwright), 5.17 (cloud runs)**.
Stage 1: decide curses shim (console frontend) vs. an own frontend replacing
`allui.c` (tiles); the port needs both text and the own tileset, so likely the
console path for game logic plus RVIP tiles from `tileset.png`. Report it.
Work stage by stage (1 → 9). Per stage: checkpoint (test, `HANDOVER.md`
`## RVIP progress`, commit `RVIP: stage N <topic>`, **push** to `origin`).
One topic per commit; never force-push.

## TSL specifics
- Keep gameplay as the original; the port adds frontend, explore/stairs keys,
  Enter menu (every command from `help.c`/keymap), tiles (own set), sound (off
  by default; the game has none: synthesized or a found free pack, RVIP 6).
- Saves/high scores: find the save/score paths in `saveload.c`, `losegame.c`,
  `wingame.c`; IDBFS (5.10); stage 9 beacon on quit, death and win.
- Shrine: happyponyland.net (may be proxy-blocked), README, CHANGES.

## Can't do from the cloud — leave for the Mac session
- Deploy to ruzzoli.de (no ssh): build `web/dist`, Playwright-test, write
  "ready to deploy" in the handover.
- Editing `RVIP.md` (other repo): reusable lessons go to `LESSONS.md` here, one
  line each, RVIP topic named.
- Publishing, repo split, visual check in the Mac browser pane (5.17).

Commit trailer: `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
