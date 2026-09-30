#!/usr/bin/env python3
"""Synthesize The Slimy Lichmummy's sound effects at build time: 3 variants per RVIP_SOUND("event")
in the game (*.c, asserted), <event>.wav, <event>-2.wav, <event>-3.wav (pitch/length
factors VARIANTS, fresh noise each), into <out>, plus <out>/sounds.json {event: [files]}
(rvip-sound.js picks one at random).
TSL ships no audio upstream, so these are made for it; events that tsl-go
(c0ze's Go port) has use its recipes (TSLGO below). Stdlib only.
Usage (repo root): python3 web/mksounds.py <out>"""
import glob, json, math, os, random, re, struct, sys, wave

R = 22050
rnd = random.Random(2012)
FK = DK = 1.0   # frequency and duration factors of the variant being rendered
VARIANTS = [(1.0, 1.0), (.94, 1.08), (1.06, .92)]

def tone(f0, f1, dur, vol=.45, dec=2.0, fm=1.0):
    """sine with a 2:1 modulator; fm = modulation depth (0 = pure sine)"""
    f0, f1, dur = f0 * FK, f1 * FK, dur * DK
    out, ph, n = [], 0.0, int(R * dur)
    for i in range(n):
        t = i / n
        ph += f0 * (f1 / f0) ** t / R
        x = math.sin(2 * math.pi * ph + fm * (1 - t) * math.sin(4 * math.pi * ph))
        out.append(x * vol * (1 - t) ** dec)
    return out

def noise(dur, vol=.5, dec=2.0, lp=.3, swell=False):
    out, y, n = [], 0.0, int(R * dur * DK)
    for i in range(n):
        t = i / n
        y += lp * (rnd.uniform(-1, 1) - y)          # one-pole low-pass: small lp = duller
        out.append(y * vol * (math.sin(math.pi * t) if swell else (1 - t) ** dec))
    return out

def mix(*parts):
    out = [0.0] * max(map(len, parts))
    for p in parts:
        for i, x in enumerate(p):
            out[i] += x
    return out

def notes(fs, d=.07, **k):
    return sum((tone(f, f, d, **k) for f in fs), [])

SOUNDS = {
    'hit':     lambda: mix(noise(.09, .7, 3, .3), tone(200, 80, .1, .4, 2, 2)),
    'kill':    lambda: mix(noise(.12, .5, 2, .2), tone(300, 60, .35, .4, 1, 3)),
    'shoot':   lambda: noise(.12, .3, lp=.8, swell=True) + tone(900, 500, .05, .15, 2, 0),
    'pickup':  lambda: tone(500, 800, .06, .3, 1, .5),
    'drop':    lambda: mix(noise(.07, .6, 4, .1), tone(150, 90, .07, .35, 3, 1)),
    'spell':   lambda: notes([988, 1319, 1661, 1976], .04, vol=.25, dec=.4, fm=2) + tone(1976, 988, .25, .2, 1.5, 1),
    'quaff':   lambda: sum((tone(f, f * 1.8, .05, .35, 1, 0) for f in (320, 410, 360, 480)), []),
    'eat':     lambda: sum((noise(.06, .5, 3, .2) + [0.0] * 1500 for _ in range(3)), []),
    'read':    lambda: noise(.12, .3, lp=.7, swell=True) + noise(.1, .25, lp=.8, swell=True),
    'hurt':    lambda: mix(noise(.14, .8, 2, .1), tone(130, 45, .14, .45, 2, 2)),
    'teleport': lambda: tone(300, 1800, .3, .3, .5, 3),
    'wear':    lambda: noise(.1, .4, 2, .15) + noise(.08, .3, 2, .2),
    'stairs':  lambda: notes([587, 554, 494, 440], .06, vol=.3, dec=.5, fm=.6),
    'death':   lambda: notes([349, 330, 311], .25, vol=.35, dec=.3, fm=1.5) + tone(294, 110, 1.0, .35, 1.2, 2),
}

# tsl-go's effects (github.com/c0ze/tsl-go web/index.html `sfx`, Web Audio
# recipes: oscillator glides and filtered noise under an 8 ms attack /
# exponential decay envelope), rendered here to wav with the same parameters.
# They replace the synthesized sound where tsl-go has an event for the same
# game action; kill, shoot, drop and teleport keep the sounds above.
def _env(i, n, peak):
    t, a = i / R, 0.008
    d = n / R
    if t < a:
        return 0.0001 * (max(peak, .0002) / 0.0001) ** (t / a)
    return max(peak, .0002) * (0.0001 / max(peak, .0002)) ** ((t - a) / max(d - a, 1e-3))

def g_tone(t0, typ='sine', f0=440, f1=None, dur=.1, gain=.2):
    t0, f0, f1, dur = t0 * DK, f0 * FK, f1 and f1 * FK, dur * DK
    n, ph, out = int(R * dur), 0.0, [0.0] * int(R * t0)
    for i in range(n):
        f = f0 * ((f1 or f0) / f0) ** (i / n)
        ph = (ph + f / R) % 1.0
        x = {'sine': math.sin(2 * math.pi * ph), 'square': 1.0 if ph < .5 else -1.0,
             'sawtooth': 2 * ph - 1, 'triangle': 4 * abs(ph - .5) - 1}[typ]
        out.append(x * _env(i, n, gain))
    return out

def g_noise(t0, dur=.1, gain=.2, typ='bandpass', freq=1000, f1=None, q=1.0):
    t0, dur, freq, f1 = t0 * DK, dur * DK, freq * FK, f1 and f1 * FK
    n, out = int(R * dur), [0.0] * int(R * t0)
    x1 = x2 = y1 = y2 = 0.0
    for i in range(n):
        f = min(freq * ((f1 or freq) / freq) ** (i / n), R * .45)
        w = 2 * math.pi * f / R; al = math.sin(w) / (2 * q); c = math.cos(w)
        if typ == 'lowpass':    b = ((1 - c) / 2, 1 - c, (1 - c) / 2)
        elif typ == 'highpass': b = ((1 + c) / 2, -(1 + c), (1 + c) / 2)
        else:                   b = (al, 0.0, -al)
        a0 = 1 + al
        x = rnd.uniform(-1, 1)
        y = (b[0] * x + b[1] * x1 + b[2] * x2 - (-2 * c) * y1 - (1 - al) * y2) / a0
        x2, x1, y2, y1 = x1, x, y1, y
        out.append(y * _env(i, n, gain))
    return out

TSLGO = {   # our event: tsl-go recipe (its gains; lowpass/highpass Q = Web Audio default)
    'hit':    lambda: mix(g_noise(0, .10, .5, 'lowpass', 2200, q=.707), g_tone(0, 'triangle', 180, 70, .12, .4)),
    'hurt':   lambda: mix(g_tone(0, 'sawtooth', 220, 80, .22, .32), g_noise(0, .16, .25, 'lowpass', 900, q=.707)),
    'death':  lambda: mix(g_tone(0, 'square', 300, 60, .34, .26), g_noise(0, .3, .2, 'lowpass', 1200, q=.707)),
    'pickup': lambda: mix(g_tone(0, 'square', 520, None, .06, .22), g_tone(.07, 'square', 780, None, .08, .22)),
    'eat':    lambda: mix(g_noise(0, .08, .3, 'lowpass', 600, q=.707), g_noise(.12, .08, .28, 'lowpass', 520, q=.707)),
    'quaff':  lambda: mix(*[g_tone(i * .07, 'sine', f, f * .8, .09, .22) for i, f in enumerate((320, 300, 360, 280))]),
    'read':   lambda: g_noise(0, .18, .18, 'highpass', 2600, q=.707),
    'wear':   lambda: mix(g_noise(0, .12, .22, 'bandpass', 2000, q=.8), g_tone(.02, 'triangle', 900, 1200, .1, .16)),
    'stairs': lambda: mix(*([g_tone(i * .1, 'triangle', f, f * .98, .16, .24) for i, f in enumerate((392, 330, 294, 247, 196))]
                            + [g_noise(.5, .18, .18, 'lowpass', 500, q=.707)])),
    'spell':  lambda: mix(g_noise(0, .30, .16, 'bandpass', 600, 3600, .6),
                          *[g_tone(.045 * i, 'sine', f, None, .2, .12) for i, f in enumerate((523, 659, 784, 1047))]),
}
def _loud(f):
    def g():
        s = f(); m = max(1e-6, max(abs(x) for x in s))
        return [x * .8 / m for x in s]          # tsl-go plays at 0.35 master; normalise the wav
    return g
SOUNDS.update({k: _loud(v) for k, v in TSLGO.items()})

events = set()
for f in glob.glob('*.c'):
    events |= set(re.findall(r'RVIP_SOUND\s*\("(\w+)"', open(f, encoding='utf-8', errors='replace').read()))
    events |= {e for p in re.findall(r'RVIP_SOUND\([^;]*\?\s*"(\w+)"\s*:\s*"(\w+)"\)', open(f, encoding='utf-8', errors='replace').read()) for e in p}
assert events and events <= set(SOUNDS), 'events without a sound: %s' % sorted(events - set(SOUNDS))

out = sys.argv[1]
os.makedirs(out, exist_ok=True)
files = {}
for ev in sorted(events):
    files[ev] = []
    for v, (FK, DK) in enumerate(VARIANTS):
        s, name = SOUNDS[ev](), ev + ('-%d' % (v + 1) if v else '') + '.wav'
        with wave.open(os.path.join(out, name), 'wb') as w:
            w.setnchannels(1); w.setsampwidth(2); w.setframerate(R)
            w.writeframes(b''.join(struct.pack('<h', int(max(-1, min(1, x)) * 32000)) for x in s))
        files[ev].append(name)
json.dump(files, open(os.path.join(out, 'sounds.json'), 'w'))
