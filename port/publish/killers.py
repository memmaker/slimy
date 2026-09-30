#!/usr/bin/env python3
"""RVIP stage 9: killer art for the graveyard (roguelikes killers/slimy/<slug>.png, 32 px).

Killer = creature->name_one minus its article, as losegame.c set_killer()
sends it; sprite = that creature's gent in the game's own tileset.png (20x20
slot at x = 1 + (g%16)*21, y = 161 + (g/16)*21, allui.c gent_rect), with the
same stand-ins as console.c map_put, nearest-neighbour to 32 px.
Run from the game folder: python3 port/publish/killers.py [outdir]
(needs gcc for the gent enum and Pillow). Same code as the make.py function
slimy() drafted in port/publish/README.md."""
import os, re, subprocess, sys
from PIL import Image

src = os.path.dirname(os.path.abspath(__file__)) + '/../..'
out = sys.argv[1] if sys.argv[1:] else os.path.join(src, 'port/publish/killers/slimy')

def slug(s): return re.sub('[^a-z0-9]', '-', s.lower())

pp = subprocess.run(['gcc', '-E', '-P', '-x', 'c', '-'], input='#include "gent.h"\n', text=True,
                    capture_output=True, cwd=src).stdout
body = re.search(r'enum\s+gent_t\s*\{(.*?)\}', pp, re.S).group(1)
gent, v = {}, -1
for e in body.split(','):
    e = e.strip()
    if not e: continue
    m = re.match(r'(\w+)\s*(?:=\s*(\w+))?', e)
    v = (gent[m.group(2)] if m.group(2) in gent else int(m.group(2), 0)) if m.group(2) else v + 1
    gent[m.group(1)] = v
stand = dict(re.findall(r'\{\s*(gent_\w+),\s*(gent_\w+)\s*\}', open(src + '/console.c').read()))

pairs = []   # (name_one, gent): a name without its own gent keeps the last one set (gnoblin archer/thief)
for f in ('monster.c', 'unique.c', 'actions.c'):
    last = None
    for m in re.finditer(r'set_creature_name\(\w+,\s*"([^"]*)"|->gent\s*=\s*(gent_\w+)|case\s+(\w+):', open(os.path.join(src, f)).read()):
        if m.group(3) and m.group(3).startswith('unique_'): last = None   # uniques each start from a blank gent
        elif m.group(2):
            last = m.group(2)
            if pairs and pairs[-1][1] is None: pairs[-1] = (pairs[-1][0], last)
        elif m.group(1): pairs.append((m.group(1), None))
    # a name followed by no gent before the next name keeps the previous gent
    for i, (n, g) in enumerate(pairs):
        if g is None and i and pairs[i - 1][1] and f == 'monster.c': pairs[i] = (n, pairs[i - 1][1])
pairs = [(n, g) for n, g in pairs if g]

img = Image.open(src + '/tileset.png').convert('RGBA')
os.makedirs(out, exist_ok=True)
done = set()
for name, g in pairs:
    n = re.sub(r'^(a|an|the|The) ', '', name)
    t = gent[stand.get(g, g)]
    x, y = 1 + (t % 16) * 21, 161 + (t // 16) * 21
    img.crop((x, y, x + 20, y + 20)).resize((32, 32), Image.NEAREST).save(os.path.join(out, slug(n) + '.png'), optimize=True)
    done.add(slug(n))
print('slimy', len(done), sorted(done))
