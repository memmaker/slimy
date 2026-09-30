# Publish prep (RVIP stages 7-9, cloud) — apply on the Mac

Nothing here is public. Publishing needs the user's decision first: TSL is not
free software (LICENSE.TXT: unmodified copies only; ports tolerated, but the
author, Ulf Åström, asks to be contacted first; no selling).

Version facts: The Slimy Lichmummy 0.40 (2012-09-26, CHANGES.TXT), upstream
https://gitlab.com/vitaly-zdanevich/the-slimy-lichmummy/-/tree/6f885be8f3f5ad640b3fc711ea5e4e56189c06e8
(2025 upload of archive.org ArchiveRL.7z; commit 1 698f45f = pristine tree).
Year 2006: backloggd.com ("released December 31, 2006") and the Roguetemple
blog ("released in 2006, last update in 2012"); CHANGES.TXT's first dated
entry is 0.3 (070922 = 2007-09-22), 0.1/0.2 undated. RogueBasin and
happyponyland.net were unreachable from the cloud: cross-check there.
Lineage: no parent (own C code, "hacklike"), top-level `<li class="insp">`.

Files (for ~/Games/roguelikes-index):
- `card.html`: card, insert in year order (`order.py --fix`); no Info button
  until the shrine is deployed, then add `<a class="play info" href="shrine/slimy.html">Info</a>`.
- `tree.html`: top-level `<li class="insp">` in `#tree` by year (2006); add
  `<a class="shrine" href="shrine/slimy.html" ...>✦</a>` with the shrine.
- `years.json.txt`: line for `years.json` `games`.
- `slimy.png` -> `img/slimy.png` (384x160, `card-image.py`: 60 sprites of the
  game's own tileset.png at 32 px, nearest-neighbour, on black).
- `og.html`: expected `<!--og-->` block (run og.py's second loop for slimy
  only; then drop web/index.html's old `<meta name="description">`).
- `killers/slimy/*.png` -> `killers/slimy/` (44 PNGs, 32 px, transparent
  where the sheet is magenta). Add to killers/make.py:
  `def slimy(): subprocess.run(['python3', G + '/slimy/port/publish/killers.py', os.path.join(HERE, 'slimy')])`
  (or paste the body of `killers.py`) and run only `make.py slimy`.
- `shrine/`: shrine page draft (stage 8), see `shrine/NOTES.md`.
- Game page: `#bar h1` -> `<a href="../shrine/slimy.html">` once the shrine is live.
