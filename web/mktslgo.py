#!/usr/bin/env python3
"""RVIP 5.8: the tsl-go tile set (github.com/c0ze/tsl-go web/sprites.png,
32x32 cells, 16 per row) as a second selectable set.

Reads web/tslgo/sprites.js (tsl-go's atlas index, vendored unchanged) and the
game's gent enum (gcc -E gent.h), maps every gent to one tsl-go sprite by the
hand tables below (DIRECT = a sprite made for that thing, STAND = a same-set
stand-in), asserts every sprite name exists and every drawn gent is mapped,
and writes port/tslgo_tiles.h for map_put (console.c). Prints coverage.
Walls and floors come from tsl-go's per-level themes (level_index = LEVEL_*),
the variant picked in C from the level cell.

Usage: python3 web/mktslgo.py   (rerun after changing a table; commit the .h)
"""
import json, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
js = open(os.path.join(ROOT, 'web/tslgo/sprites.js')).read()
SP = json.loads(js[js.index('{'):js.rindex('}') + 1])
CELL = SP['cell']
def slot(name):
    assert name in SP['sprites'], 'no tsl-go sprite ' + name
    x, y = SP['sprites'][name]['f'][0]
    s = (y // CELL) * 16 + x // CELL
    assert s < 250
    return s

WALL, FLOOR = 254, 253          # markers: the level's wall / floor family
DIRECT = {
    'floor': FLOOR, 'door_closed': 'door_closed', 'door_open': 'door_open',
    'water': 'water_a', 'lava': 'lava_a', 'stairs': 'stairs_down', 'ice': 'floor_ice_0',
    'dart_trap': 'trap_dart', 'plate_trap': 'trap_plate', 'polymorph_trap': 'trap_poly',
    'flash_trap': 'flash_trap', 'web': 'web_trap',
    'player': 'player', 'ghoul': 'ghoul', 'chrome_angel': 'm_chrome_angel',
    'burning_skull': 'burning_skull', 'crypt_vermin': 'crypt_vermin',
    'severed_hand_m': 'severed_hand', 'graveling': 'graveling', 'gnoblin': 'm_gnoblin',
    'hellhound': 'm_hellhound', 'flame_spirit': 'flame_spirit', 'floating_brain': 'm_brain',
    'wolf': 'dire_wolf', 'frostling': 'frostling', 'imp': 'imp',
    'giant_slimy_toad': 'm_toad', 'chainsaw_ogre': 'chainsaw_ogre', 'sentinel': 'm_sentinel',
    'elder_mummylich': 'm_mummylich', 'nameless_horror': 'm_horror', 'scarecrow': 'scarecrow',
    'merman': 'm_merman', 'slime': 'm_slime', 'goatman': 'goatman', 'gloom_lord': 'm_gloom_lord',
    'ratman': 'ratman', 'tentacle': 'tentacle', 'technician': 'technician', 'mimic': 'mimic',
    'electric_snake': 'm_electric_snake', 'sludge_dweller': 'sludge_dweller', 'lurker': 'lurker',
    'dragon': 'm_dragon', 'gaoler': 'gaoler', 'king_of_worms': 'king_of_worms',
    'necromancer': 'm_necromancer', 'phantasm': 'wisp', 'drowned_one': 'zombie',
    'sword': 'weapon_doom', 'staff': 'weapon_staff', 'bow': 'bow', 'arrow': 'arrows',
    'light_armor': 'armor_leather', 'heavy_armor': 'armor_chain', 'boots': 'boots',
    'cloak': 'cloak', 'robe': 'armor_rune', 'amulet': 'amulet', 'potion': 'potion_ruby',
    'scroll': 'scroll_grey', 'book': 'book_red', 'wand': 'wand_wood', 'torch': 'torch',
    'mushroom': 'mushroom', 'key': 'key', 'meat': 'food_ration',
    'severed_hand': 'severed_hand', 'corpse': 'corpse',
}
STAND = {   # same tsl-go set, nearest thing
    'obstacle': WALL, 'block': WALL, 'wall_v': WALL, 'wall_h': WALL, 'wall_es': WALL,
    'wall_sw': WALL, 'wall_ne': WALL, 'wall_nw': WALL, 'wall_nes': WALL, 'wall_nsw': WALL,
    'wall_new': WALL, 'wall_esw': WALL, 'wall_cross': WALL,
    'pentagram': 'altar', 'capsule': 'wall_lab_5', 'terminal': 'wall_lab_6',
    'forcefield': 'wall_hub_0', 'generator': 'wall_lab_0', 'internal_trap': 'trap_plate',
    'medkit': 'potion_white', 'trap_generic': 'trap_plate', 'booby_trap': 'trap_plate',
    'blink_trap': 'trap_poly', 'glass_trap': 'trap_plate',
    'f_d_g': 'm_brain', 'caeltzan': 'm_necromancer', 'ybznek': 'm_horror', 'lognac': 'goatman',
    'spear': 'weapon_staff', 'axe': 'weapon_crystal', 'club': 'weapon_staff',
    'whip': 'weapon_staff', 'shotgun': 'bow', 'shell': 'arrows', 'pistol': 'bow',
    'bullet': 'arrows', 'crossbow': 'bow', 'dart': 'arrows', 'blowgun': 'bow',
    'grenade': 'potion_black', 'goggles': 'helmet', 'crown': 'hat', 'gas_mask': 'helmet',
    'dataprobe': 'wand_iron', 'lantern': 'torch', 'fire_extinguisher': 'potion_brown',
    'mortar_pestle': 'potion_murky', 'hacksaw': 'weapon_dagger', 'bone': 'corpse',
    'prod': 'weapon_staff', 'cheese': 'food_ration', 'bread': 'food_ration',
    'fish': 'food_ration', 'chickpeas': 'food_ration', 'falafel': 'food_ration',
    'sausage': 'food_ration', 'decapitated_head': 'burning_skull', 'ogre_corpse': 'corpse',
    'carcass': 'corpse', 'eyeball': 'm_brain', 'cranium': 'burning_skull',
    'bone_dust': 'corpse', 'mandrake_root': 'mushroom', 'beetle_shell': 'corpse',
    'mummy_wrapping': 'armor_rune',
    **{'arrow_' + d: 'arrows' for d in ('n', 'ne', 'e', 'se', 's', 'sw', 'w', 'nw')},
    **{'spell_' + d: 'wisp' for d in ('n', 'ne', 'e', 'se', 's', 'sw', 'w', 'nw')},
    'spell_poison': 'm_slime', 'spell_fire': 'flame_spirit', 'spell_frost': 'frostling',
    'explosion': 'flame_spirit', 'flash': 'flash_trap',
}
NOT_DRAWN = {'undefined', 'blank', 'max'}    # blank = unknown cell, stays black

src = subprocess.run(['gcc', '-E', '-P', '-DTSL_CONSOLE', '-I' + ROOT + '/port', ROOT + '/gent.h'],
                     capture_output=True, text=True, check=True).stdout
enum = re.search(r'enum gent_t\s*\{([^}]*)\}', src).group(1)
gents = [re.sub(r'=.*', '', e).strip()[5:] for e in enum.split(',') if e.strip()]
drawn = [g for g in gents if g not in NOT_DRAWN]
missing = [g for g in drawn if g not in DIRECT and g not in STAND]
assert not missing, 'unmapped gents: ' + ' '.join(missing)
assert not set(DIRECT) & set(STAND)

THEMES = {  # LEVEL_* (places.h) -> tsl-go level id
    'DUNGEON': 'dungeon', 'OMINOUS_CAVE': 'ominous_cave', 'DROWNED_CITY': 'drowned_city',
    'CATACOMBS': 'catacombs', 'DRAGONS_LAIR': 'dragons_lair', 'FROZEN_VAULT': 'frozen_vault',
    'CHAPEL': 'chapel', 'LABORATORY': 'laboratory', 'COMM_HUB': 'comm_hub',
    'UNDERPASS': 'underpass', 'TEST': 'dungeon'}

def val(v): return v if isinstance(v, int) else slot(v)
out = ['/* Generated by web/mktslgo.py from web/tslgo/sprites.js (tsl-go) - do not edit. */',
       '/* tsl-go sprite slot per gent (sheet web/tslgo/sprites.png, 32x32, 16 per row);',
       '   254 = the level\'s wall family, 253 = its floor family, 255 = none. */',
       'static const unsigned char tslgo_gent[gent_max] = {']
for g in drawn:
    out.append('  [gent_%s] = %d,' % (g, val(DIRECT.get(g, STAND.get(g)))))
out.append('};')
for kind in ('wall', 'floor'):
    out.append('static const unsigned char tslgo_%s[LEVELS][%d] = {' % (kind, 10))
    for lv, tid in THEMES.items():
        s = [slot(n) for n in SP['themes'][tid][kind]][:10]
        out.append('  [LEVEL_%s] = { %s },' % (lv, ', '.join(map(str, s + [255] * (10 - len(s))))))
    out.append('};')
open(os.path.join(ROOT, 'port/tslgo_tiles.h'), 'w').write('\n'.join(out) + '\n')
nd = sum(1 for g in drawn if g in DIRECT)
print('tsl-go coverage: %d drawn gents, %d own sprite (%d%%), %d same-set stand-in, 100%% with stand-ins'
      % (len(drawn), nd, round(100 * nd / len(drawn)), len(drawn) - nd))
