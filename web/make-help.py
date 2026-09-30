#!/usr/bin/env python3
"""Writes the in-page game guide (dist/help.html) for the web build of
The Slimy Lichmummy. Self-contained (cloud run, RVIP 5.12): the game text is
TSL's manual (README.md) and its in-game help pages (help.c) in our own
words; keys are the default keymap (keymap.c common_keys/default_keymap)
plus the port's additions. Shaped like a Docs GAMES/GUIDES entry so the
Mac session can move it into build-docs.py / guides.py.
Usage (repo root): python3 web/make-help.py <out>"""
import html, sys

esc = html.escape


def kbd(k):
    return '<span class="or">or</span>'.join('<kbd>%s</kbd>' % esc(x) for x in k.split(' / '))


TAGLINE = ('A dark, fast roguelike by Ulf Åström (2012): descend through the underworld, '
           'improve yourself with augmentations and facets, and fight with melee, '
           'ranged weapons, wands and abilities.')

ABOUT = '''<p><strong>The Slimy Lichmummy</strong> (TSL) is a turn-based roguelike: every
action you take lets the monsters act, the dungeon is made anew for every game
and death is permanent. There is no character creation: you start straight in
the dungeon. Instead of experience levels you grow by entering augmentation
<em>capsules</em> (upgrades to your body that stay for the rest of the game) and
by earning <em>facets</em> through achievements. Abilities (spells and skills)
cost energy, which comes back over time.</p>
<p>Movement and ranged combat are one system: <kbd>Shift</kbd> + a direction fires
your weapon, zaps the wand you hold or throws your ammunition that way.</p>'''

KEY_HINTS = [
    ('?', "The game's own help pages (key reference, legend, movement, items, combat…)"),
    ('x', 'Auto-explore: walk towards unexplored places; stops when a creature or new item comes into view, when something happens, or on any key'),
    ('Enter', 'Menu of all commands (choose with the arrows and Enter, or press the command\'s key)'),
    ('i', 'Inventory with a cursor: a letter does the item\'s main action, Shift+letter drops it, Enter opens a menu of what you can do with it'),
    ('< / >', 'Walk to the nearest known staircase up / down and stop on it; press again there to climb'),
    ('S', 'Save and quit (the next visit continues the game)'),
]

ESSENTIALS = [
    ('Moving', [
        ('h j k l', 'Move west, south, north, east'),
        ('y u b n', 'Move diagonally'),
        ('Arrows / Numpad', 'Move (keypad 7/9/1/3 diagonally)'),
        ('.', 'Wait one turn'),
        ('T', 'Rest until bad effects expire or an enemy appears'),
        ('x', 'Auto-explore'),
        ('< / >', 'Stairs (walk to known stairs; climb when on them)'),
    ]),
    ('Fighting', [
        ('move into', 'Melee attack (bump)'),
        ('H J K L Y U B N', 'Fire in a direction (Shift + move key)'),
        ('Shift+Arrows', 'Fire in a direction'),
        ('f', 'Fire'),
        ('t', 'Throw an item'),
        ('W', 'Cycle ammunition (free action)'),
        ('Z', 'Use an ability'),
        ('1 … 0', 'Ability shortcuts'),
    ]),
    ('Items', [
        ('i', 'Browse inventory'),
        (', / g', 'Pick up'),
        ('d', 'Drop'),
        ('e / r', 'Equip / remove'),
        ('D / E / R', 'Drink / eat / read'),
        ('p', 'Zap a wand or apply a tool'),
        ('a', 'Use an item'),
    ]),
    ('Game', [
        ('Enter', 'Command menu'),
        ('Space / Esc', 'Cancel, leave a menu'),
        ('Tab', 'Switch the status display'),
        ('@', 'Character summary'),
        ('P', 'Message history'),
        ('S', 'Save and quit'),
        ('?', 'Help pages'),
    ]),
]

ALL = [
    ('h / Left', 'Move west'), ('j / Down', 'Move south'), ('k / Up', 'Move north'), ('l / Right', 'Move east'),
    ('y', 'Move north-west'), ('u', 'Move north-east'), ('b', 'Move south-west'), ('n', 'Move south-east'),
    ('Numpad 1-9', 'Move (Numpad 5: do what I mean)'),
    ('H J K L Y U B N', 'Fire in that direction'), ('Shift+Arrows', 'Fire in that direction'),
    ('.', 'Wait (pass) one turn'), ('T', 'Rest'), ('x', 'Auto-explore (web port)'),
    ('< / >', 'Walk to the nearest known stairs up / down; climb when on them (web port)'),
    ('c', 'Traverse stairs'), ('O', 'Close an adjacent door'),
    ('i', 'Browse inventory (letter: main action, Shift+letter: drop, Enter: item menu)'),
    (', / g', 'Pick up'), ('d', 'Drop'), ('e', 'Equip'), ('r', 'Remove'), ('D', 'Drink'), ('E', 'Eat'),
    ('R', 'Read'), ('a', 'Use an item'), ('p', 'Zap a wand or apply a tool'), ('A', 'Label an item (notes)'),
    ('f', 'Fire missile'), ('t', 'Throw an item'), ('W', 'Cycle ammo (free action)'),
    ('Z', 'Use an ability'), ('z', 'Assign ability shortcuts'), ('1 … 0', 'Ability shortcuts'),
    ('/ / I / v', 'Inspect a tile (or scroll the map view)'),
    ('Enter', 'Command menu (web port)'), ('Numpad 5', 'Confirm, do what I mean'),
    ('Space / Esc / Numpad 0', 'Cancel, clear the message bar'),
    ('+ / - / PgDn / PgUp', 'Next / previous page'),
    ('Tab', 'Flip the status display'), ('@', 'Character summary'), ('P', 'Message history'),
    ('=', 'Options'), ('C', 'Recenter view'), ('X', 'Redraw the screen'), ('V', 'Display version'),
    ('? / F1', 'Help'), ('S', 'Save and quit'), ('Q', 'Forfeit the game'),
]

SAVING = '''<ul>
<li><kbd>S</kbd> saves and quits. The save is kept in this browser (IndexedDB); the page
then starts again and TSL loads the save, continuing where you left off.</li>
<li>As in the original, <strong>loading a save deletes it</strong>: there is exactly one
save. The web version also autosaves at the start and on every level change, so a closed
or crashed page continues from the last level change; a death, quit or win removes it.</li>
<li><em>File ▾ → Export save</em> downloads the save file (only while one exists),
<em>Import save</em> loads one, <em>New game</em> deletes it. TSL's manual considers
backup copies cheating; that choice is yours.</li>
<li>When you die the game shows how it ended, then a new game starts.</li>
</ul>'''

TIPS = '''<ul>
<li>Light matters: the dungeon is dark, light sources burn out, so keep spares.</li>
<li>Most items are unidentified; potions and scrolls have random looks each game.
Label (<kbd>A</kbd>) what you learn.</li>
<li>Armour absorbs damage but wears out; the first point of every hit always gets through.</li>
<li>Daggers and similar weapons can backstab enemies that have not noticed you: a
silent instant kill. Sneaking up on sleeping monsters pays.</li>
<li>Health has no maximum: medkits keep adding to it.</li>
<li>Avoiding a fight is often the best tactic. Stunning, wounding and poison
weapons weaken what you cannot avoid.</li>
</ul>'''

GUIDE = [
    ('The screen', '<p>The map shows remembered walls, doors and traps; creatures and items only while in view. The status window lists your character (<kbd>Tab</kbd> switches pages) and doubles as the inventory list; messages appear below the map (<kbd>P</kbd> for the history).</p>'),
    ('Moving and doors', '<p>Walk with the vi-keys, the arrows or the keypad. Doors open when you walk into them; some need keys or can be broken. Walk into a wall once to search it for secret doors.</p>'),
    ('Your character', '<p><strong>Health</strong> is how much damage you can take. <strong>Energy</strong> pays for abilities and refills over time. <strong>Speed</strong> decides how often you act. <strong>Attack</strong> and <strong>dodge</strong> are percentages. <strong>Stealth</strong> keeps you unnoticed, <strong>perception</strong> finds monsters and traps, <strong>vision</strong> is how far you see.</p>'),
    ('Getting stronger', '<p>Enter augmentation capsules to install upgrades; facets come from achievements. Abilities get shortcut keys automatically; <kbd>z</kbd> shows what each does and rebinds them.</p>'),
    ('Hazards', '<p>Traps trigger when stepped on, webs hold you, lava and water hurt (swimming skill gives some free turns). Forcefields destroy most things passing through; hack or blow up their generators. Terminals can be hacked with a dataprobe or the Interface ability.</p>'),
]

WEB = '''<ul>
<li>The map uses TSL's own tiles; <em>Tiles</em> in the top bar cycles TSL → tsl-go
(the sprites of <a href="https://github.com/c0ze/tsl-go">c0ze/tsl-go</a>, a Go port of TSL)
→ None (text). The choice is remembered. Map, Messages, Status, Inventory, Visible
(creatures and items in view) and Message log are separate windows; <em>A−</em> / <em>A+</em>
on a title bar change that window's text or tile size, <em>Windows</em> arranges them or
switches to <em>One window</em>: TSL's whole text screen as in a terminal. Menus, help pages and
the death screen appear as text boxes over the map.</li>
<li><strong>Added for the web:</strong> <kbd>x</kbd> auto-explore, <kbd>&lt;</kbd> / <kbd>&gt;</kbd>
walk to the nearest known stairs, <kbd>Enter</kbd> opens a menu of every command (keypad
<kbd>5</kbd> keeps TSL's do-what-I-mean), and the inventory (<kbd>i</kbd>) takes a letter for
the main action, Shift+letter to drop, Enter for the item menu. The keypad works with
NumLock on or off.</li>
<li><strong>Audio ▾:</strong> sound effects (TSL has no sound of its own): most are
tsl-go's effects, the rest made for this port; <em>Music</em> plays tsl-go's recorded
track for each level. Both off by default; the choice is remembered.</li>
<li>Browsers keep some shortcuts (<kbd>Ctrl+W</kbd>, <kbd>Ctrl+T</kbd>, <kbd>Cmd</kbd> keys on
a Mac) for themselves.</li>
<li>If the game crashes, a message appears at the top; reload the page.</li>
</ul>'''

VERSION = '''<ul>
<li>Based on <strong>The Slimy Lichmummy 0.40</strong> by Ulf Åström
(<a href="http://happyponyland.net/">happyponyland.net</a>), from the source kept at
<a href="https://gitlab.com/vitaly-zdanevich/the-slimy-lichmummy">gitlab.com/vitaly-zdanevich/the-slimy-lichmummy</a>.
Tiles and fonts are the game's own (fonts: modified Terminus, SIL OFL).</li>
<li>The second tile set is from <a href="https://github.com/c0ze/tsl-go">c0ze/tsl-go</a>
by c0ze (tsl.coze.org): tiles from the Dungeon Crawl Stone Soup tileset (CC0; thanks to
the Crawl and Crawl Stone Soup teams, Eino Keskitalo, David Lawrence Ramsey, Enne Walker,
Poor_Yurik, Stefan O'Rear and the original RLTiles) plus sprites made for tsl-go. Sound effects (hit, hurt, death, pickup, eat, quaff,
read, wear, stairs, spell) and the level music are tsl-go's too.</li>
<li>TSL is not free software: its licence allows redistributing the unmodified
source and asks that unofficial ports contact the author. Sound effects were
synthesized for this port. Built with Emscripten.</li>
</ul>'''


def dl(items):
    return '<dl>' + ''.join('<dt>%s</dt><dd>%s</dd>' % (kbd(k), esc(d)) for k, d in items) + '</dl>'


def section(anchor, title, body):
    return '<h2 id="h-%s">%s</h2>%s' % (anchor, esc(title), body)


toc = [('about', 'About the game'), ('keys', 'Keyboard controls'), ('saving', 'Saving your game'),
       ('tips', 'Tips'), ('guide', "New player's guide"), ('web', 'Playing in the browser'),
       ('version', 'About this version')]
parts = ['<p>' + esc(TAGLINE) + '</p><ul class="toc">' +
         ''.join('<li><a href="#h-%s">%s</a></li>' % (a, esc(t)) for a, t in toc) + '</ul>']
parts.append(section('about', 'About the game', ABOUT))
ess = ''.join('<div class="box"><h3>%s</h3>%s</div>' % (esc(c), dl(i)) for c, i in ESSENTIALS)
full = ''.join('<div>%s<span>%s</span></div>' % (kbd(k), esc(d)) for k, d in ALL)
parts.append(section('keys', 'Keyboard controls',
                     '<div class="box key"><h3>The keys to remember</h3>' + dl(KEY_HINTS) + '</div>'
                     '<h3>Essential keys</h3><div class="grid">' + ess + '</div>'
                     '<details><summary>Complete key list (%d commands)</summary>' % len(ALL) +
                     '<div class="all">' + full + '</div></details>'))
parts.append(section('saving', 'Saving your game', SAVING))
parts.append(section('tips', 'Tips', TIPS))
parts.append(section('guide', "New player's guide", ''.join('<h3>%s</h3>%s' % (esc(t), b) for t, b in GUIDE)))
parts.append(section('web', 'Playing in the browser', WEB))
parts.append(section('version', 'About this version', VERSION))
text = '\n'.join(parts)
if len(sys.argv) > 1:
    open(sys.argv[1], 'w').write(text)
else:
    print(text)
