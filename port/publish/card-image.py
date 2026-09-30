#!/usr/bin/env python3
"""RVIP stage 7: card image img/slimy.png (384x160) = 12x5 sprites sampled evenly
from the game's own tileset.png gent slots (20x20 at x = 1 + (g%16)*21,
y = 161 + (g/16)*21), blank/dark slots skipped, nearest-neighbour to 32 px.
Run from the game folder: python3 port/publish/card-image.py [out.png]"""
import os, sys
from PIL import Image
src = os.path.dirname(os.path.abspath(__file__)) + '/../..'
out = sys.argv[1] if sys.argv[1:] else os.path.join(src, 'port/publish/slimy.png')
img = Image.open(src + '/tileset.png').convert('RGBA')   # palette transparency (magenta) -> alpha
slots = []
for g in range(16 * ((img.height - 161) // 21)):
    x, y = 1 + (g % 16) * 21, 161 + (g // 16) * 21
    t = Image.new('RGBA', (20, 20), 'black'); t.alpha_composite(img.crop((x, y, x + 20, y + 20))); t = t.convert('RGB')
    px = list(t.convert('L').getdata())
    lit = sum(1 for p in px if p > 60)
    if 70 <= lit <= 330 and sum(px) / len(px) >= 28: slots.append(t)
pick = [slots[i * len(slots) // 60] for i in range(60)]
card = Image.new('RGB', (384, 160))
for i, t in enumerate(pick): card.paste(t.resize((32, 32), Image.NEAREST), (i % 12 * 32, i // 12 * 32))
card.save(out, optimize=True); print(out, len(slots), 'usable slots')
