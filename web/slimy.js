/* The Slimy Lichmummy: RVIP page code (windows via ../rvip-wm.js, saves via ../rvip-app.js).
 * The game (port/wcurses.c) sends each curses window as its own pane of HTML lines:
 * board -> Map, message bar -> Messages, status -> Status (also the inventory browser),
 * stdscr -> pop-up over the map (command menu, help, item menus, death screen).
 * The map cells' tiles come from C (map_put, rule 7), one code array per set, -1 = text:
 *   own set 'Tiles' (tileset.png): top | under<<10 | sheet<<20;
 *   'tsl-go' (tslgo-sprites.png, 32 px, 16 per row): top | under<<8 | dim<<16, 255 = none. */
(function () {
	'use strict';
	function $(id) { return document.getElementById(id); }
	var DIR = RvipApp.dir, LAYOUT = DIR + '/web-layout.json', SAVE = DIR + '/TSL-SAVE';
	var TILE = 20, L = null, wm = null, app = null, rects = {};
	var SETS = { Tiles: ['tileset.png', 'tiledim.png', 'tilerev.png'], 'tsl-go': ['tslgo-sprites.png'] }, ORDER = ['Tiles', 'tsl-go', 'None'];
	var sheets = { Tiles: [], 'tsl-go': [] }, ready = {}, tileGen = 0, lastMap = null;

	function status(s, err) { if (app) app.status(s, err); else { var e = $('status'); e.textContent = s; e.hidden = !s; } }

	/* ---------- settings: one file in the game's IDBFS folder (no localStorage) ---------- */
	function loadLayout() {
		var d = { tiles: 'Tiles', face: '', mapFace: '', audio: { sound: false, music: false }, wm: null };
		try {
			var s = JSON.parse(Module.FS.readFile(LAYOUT, { encoding: 'utf8' }));
			if (ORDER.indexOf(s.tiles) >= 0) d.tiles = s.tiles;
			if (typeof s.face === 'string') d.face = s.face;
			if (typeof s.mapFace === 'string') d.mapFace = s.mapFace;
			if (s.audio) d.audio = { sound: s.audio.sound === true, music: s.audio.music === true };
			if (s.wm) d.wm = s.wm;
			/* layouts saved before the Inventory/Message log windows: take the new default */
			if (d.wm && d.wm.multi && JSON.stringify(d.wm.multi).indexOf('"vis"') < 0) delete d.wm.multi;
		} catch (e) { /* nothing saved yet */ }
		L = d;
	}
	var saveT = 0;
	function saveLayout() {
		clearTimeout(saveT);
		saveT = setTimeout(function () {
			try { Module.FS.writeFile(LAYOUT, JSON.stringify(L)); app.sync(); } catch (e) { console.warn('layout not saved', e); }
		}, 300);
	}

	/* ---------- map ---------- */
	var pairCol = ['#ccc', '#c90', '#46f', '#e33', '#3c3', '#3cc', '#c3c', '#666'];
	var acsCh = { 0xE000: '█', 0xE001: '·', 0xE002: '│', 0xE003: '─' };
	/* A−/A+ on the Map: the WM's size (px) is the text-mode font; tiles step in whole multiples */
	function mapFs() { return RvipWM.fontSize('map'); }
	function scale() { return Math.max(1, Math.min(4, Math.round(mapFs()) - 15)); }
	function tilesOn() { return L && L.tiles !== 'None' && ready[L.tiles]; }
	function cellPx() { return L && L.tiles === 'tsl-go' ? 32 : TILE; }
	function drawMap() {
		var c = $('mapc');
		if (!lastMap || !tilesOn()) return;
		var go = L.tiles === 'tsl-go', t = go ? lastMap.t2 : lastMap.t, cells = lastMap.c, h = lastMap.h, w = lastMap.w, S = cellPx() * scale();
		if (c.width !== w * S || c.height !== h * S) { c.width = w * S; c.height = h * S; }
		var g = c.getContext('2d');
		g.imageSmoothingEnabled = false;
		g.fillStyle = '#000'; g.fillRect(0, 0, c.width, c.height);
		g.font = (S - 4) + 'px monospace'; g.textAlign = 'center'; g.textBaseline = 'middle';
		for (var y = 0; y < h; y++) for (var x = 0; x < w; x++) {
			var i = y * w + x, code = t[i], dx = x * S, dy = y * S;
			if (code >= 0 && go) {
				var gs = sheets['tsl-go'][0], gt = code & 255, gu = (code >> 8) & 255;
				if (gu !== 255) g.drawImage(gs, (gu % 16) * 32, (gu >> 4) * 32, 32, 32, dx, dy, S, S);
				if (gt !== 255 && gt !== gu) g.drawImage(gs, (gt % 16) * 32, (gt >> 4) * 32, 32, 32, dx, dy, S, S);
				if (code & 0x10000) { g.fillStyle = 'rgba(0,0,0,0.55)'; g.fillRect(dx, dy, S, S); }
			} else if (code >= 0) {
				var sh = sheets.Tiles[(code >> 20) & 3], top = code & 1023, und = (code >> 10) & 1023;
				g.drawImage(sh, 1 + (und % 16) * 21, 161 + ((und / 16) | 0) * 21, TILE, TILE, dx, dy, S, S);
				if (top !== und) g.drawImage(sh, 1 + (top % 16) * 21, 161 + ((top / 16) | 0) * 21, TILE, TILE, dx, dy, S, S);
			} else {
				var cell = cells[i], ch = cell & 0xffff;
				if (ch === 32 && !(cell & 0x100000)) continue;
				var col = pairCol[(cell >> 16) & 7];
				if (cell & 0x100000) { g.fillStyle = col; g.fillRect(dx, dy, S, S); col = '#000'; }
				g.fillStyle = col;
				g.fillText(ch >= 0xE000 ? (acsCh[ch] || '?') : String.fromCharCode(ch), dx + S / 2, dy + S / 2 + 1);
			}
		}
	}
	/* the camera: the hero's cell (from C) centred, clamped; a smaller map is centred */
	function camera() {
		var on = tilesOn(), el = on ? $('mapc') : $('map');
		$('mapc').hidden = !on; $('map').hidden = on;
		if (!lastMap) return;
		var cw, ch, w, h;
		if (on) { cw = ch = cellPx() * scale(); w = lastMap.w * cw; h = lastMap.h * ch; }
		else {
			var m = $('map'); m.style.width = m.style.height = '';
			var r = cellSize(); cw = r.w; ch = r.h; w = lastMap.w * cw; h = lastMap.h * ch;
			m.style.width = w + 'px'; m.style.height = h + 'px';
		}
		var hy = lastMap.hy >= 0 ? lastMap.hy : lastMap.h / 2, hx = lastMap.hx >= 0 ? lastMap.hx : lastMap.w / 2;
		RvipWM.center(el, (hx + 0.5) * cw, (hy + 0.5) * ch, w, h);
	}
	var meas = null;
	function cellSize() {
		if (!meas) { meas = document.createElement('span'); meas.textContent = 'MMMMMMMMMM'; meas.style.visibility = 'hidden'; }
		$('map').appendChild(meas);
		var r = { w: meas.offsetWidth / 10, h: meas.offsetHeight };
		meas.remove();
		return r;
	}
	function redrawMap() { drawMap(); camera(); drawVis(); }

	/* ---------- tiles: the game's own tileset.png (+ dim/rev sheets), chosen by name ---------- */
	/* sets by name: 'Tiles' = the game's own, 'tsl-go' = c0ze/tsl-go's sprites, 'None' = text */
	function loadSheets(done) {
		var set = L.tiles, gen = ++tileGen, names = SETS[set], left = names.length;
		if (!names || ready[set]) { if (done) done(); redrawMap(); return; }
		names.forEach(function (n, k) {
			var im = sheets[set][k] = new Image();
			im.onload = function () { if (--left === 0) { ready[set] = true; if (gen === tileGen) { if (done) done(); redrawMap(); } } };
			im.onerror = function () { if (gen === tileGen) { status('Could not load the tile set; using text.', true); if (done) done(); } };
			im.src = n;
		});
	}
	function renderTiles() { $('btn-tiles').textContent = 'Tiles: ' + (L ? (L.tiles === 'Tiles' ? 'TSL' : L.tiles) : 'TSL'); renderMapSel(); }
	function toggleTiles() {
		L.tiles = ORDER[(ORDER.indexOf(L.tiles) + 1) % ORDER.length]; saveLayout(); renderTiles();
		if (L.tiles !== 'None') loadSheets(); else { tileGen++; redrawMap(); }
	}

	/* ---------- fonts ---------- */
	var mapSel = document.createElement('select');
	mapSel.title = 'Map font (text mode)';
	mapSel.innerHTML = '<option value="">Default font</option>';
	mapSel.addEventListener('pointerdown', function (e) { e.stopPropagation(); });
	function renderMapSel() {
		var bs = document.querySelector('#t-map .wm-btns');
		if (bs && mapSel.parentNode !== bs) bs.insertBefore(mapSel, bs.firstChild);
		mapSel.hidden = !!(L && L.tiles !== 'None');
		mapSel.value = (L && L.mapFace) || '';
	}
	function face(n) { return n ? '"' + n + '", ui-monospace, Menlo, monospace' : 'ui-monospace, Menlo, monospace'; }
	function applyFace() {
		['#t-stat .body', '#t-msg .body', '#inv', '#vis', '#log', '#pop'].forEach(function (q) { var e = document.querySelector(q); if (e) e.style.fontFamily = face(L.face); });
		$('map').style.fontFamily = face(L.mapFace);
		camera();
	}
	function loadFace(n) {
		if (!n) { applyFace(); return; }
		var ff = new FontFace(n, 'url(../fonts/' + n + '.woff)');
		ff.load().then(function () { document.fonts.add(ff); applyFace(); }).catch(function () { status('Could not load the font ' + n + '.', true); });
	}

	/* ---------- Visible window: RvipWM.visible, icons from the chosen set (tile = own gent | tsl-go << 10) ---------- */
	var lastVis = '';
	function visIcon(t) {
		if (!tilesOn()) return null;
		var d = document.createElement('span'), go = L.tiles === 'tsl-go', n = go ? t >> 10 : t & 1023;
		if (go && n === 255) return null;
		d.style.cssText = 'display:inline-block;width:16px;height:16px;vertical-align:-3px;image-rendering:pixelated;background-repeat:no-repeat;' +
			(go ? 'background-image:url(tslgo-sprites.png);background-size:256px 240px;background-position:' + -(n % 16) * 16 + 'px ' + -(n >> 4) * 16 + 'px'
				: 'background-image:url(tileset.png);background-size:269.6px 666.4px;background-position:' + -(1 + (n % 16) * 21) * 0.8 + 'px ' + -(161 + (n >> 4) * 21) * 0.8 + 'px');
		return d;
	}
	var visSet;
	function drawVis() {   /* RvipWM.visible skips an unchanged list; a set switch forces the icons */
		var b = $('vis'), set = tilesOn() ? L.tiles : '';
		if (set !== visSet) { visSet = set; b._vis = null; }
		RvipWM.visible(b, lastVis, visIcon);
	}

	/* ---------- pop-up: stdscr (menus, help, item lists), text follows Messages' size ---------- */
	function popShow(on) {
		var p = $('pop');
		p.hidden = !on;
		if (on) { p.style.fontSize = RvipWM.fontSize('msg') + 'px'; RvipWM.popup(p); }
	}

	/* ---------- windows ---------- */
	function makeWM() {
		wm = RvipWM({
			area: $('game'), menu: $('btn-layout'),
			wins: [{ id: 'map', title: 'Map' }, { id: 'msg', title: 'Messages' }, { id: 'stat', title: 'Status' },
				{ id: 'inv', title: 'Inventory' }, { id: 'vis', title: 'Visible' }, { id: 'log', title: 'Message log' }],
			multi: { d: 'h', r: 0.64, a: { d: 'v', r: 0.8, a: 'map', b: 'msg' }, b: { d: 'v', r: 0.4, a: 'stat', b: { d: 'v', r: 0.36, a: 'inv', b: { d: 'v', r: 0.5, a: 'vis', b: 'log' } } } },
			single: { d: 'h', r: 0.64, a: { d: 'v', r: 0.8, a: 'map', b: 'msg' }, b: 'stat' },
			state: L.wm,
			save: function (st) { L.wm = st; saveLayout(); },
			layout: function (r) { rects = r; redrawMap(); if (!$('pop').hidden) popShow(true); var mb = $('msg').parentNode; mb.scrollTop = mb.scrollHeight; },
			zoom: { map: function () { redrawMap(); }, msg: function () { if (!$('pop').hidden) popShow(true); } },
			size: { map: function () { return 16; } },
			fontMax: { map: 19 },
			onReset: function () { redrawMap(); }
		});
		wm.apply();
		renderMapSel();
	}

	/* ---------- sound ---------- */
	/* events come from game actions (RVIP_SOUND in the C code -> web_sound ->
	 * Module.rvipSound); web/mksounds.py synthesizes 3 wavs per event,
	 * rvip-sound.js plays them. Off by default; nothing is fetched until
	 * Sound effects is on. Events tsl-go has use its recipes (mksounds.py).
	 * Music: tsl-go's recorded track per level (music/<level>.mp3), off by
	 * default; the Audio element is made only when Music goes on. */
	var LEVELS = [null, 'dungeon', 'ominous_cave', 'drowned_city', 'catacombs', 'dragons_lair',
		'frozen_vault', 'chapel', 'laboratory', 'comm_hub', 'underpass'];   /* places.h LEVEL_* */
	var music = { el: null, level: -1, src: '' };
	window.slimyMusic = function () { return music; };
	function musicUpdate() {
		var want = L && L.audio.music && LEVELS[music.level] ? 'music/' + LEVELS[music.level] + '.mp3' : '';
		if (!want) { if (music.el) music.el.pause(); return; }
		if (!music.el) { music.el = new Audio(); music.el.loop = true; music.el.volume = 0.4; }
		if (music.src !== want) { music.src = want; music.el.src = want; }
		music.el.play().catch(function () { /* autoplay: retried on the next key/click */ });
	}
	['keydown', 'pointerdown'].forEach(function (t) {
		document.addEventListener(t, function () { if (music.el && music.el.paused && L && L.audio.music && music.src) music.el.play().catch(function () {}); });
	});
	var audio = { cfg: null, loading: false, played: 0 };
	window.slimyAudio = function () { return audio; };
	function sound(name) {
		if (!L || !L.audio.sound) return;
		if (!audio.cfg) {
			if (!audio.loading) {
				audio.loading = true;
				fetch('sound/sounds.json').then(function (r) { return r.json(); })
					.then(function (c) { audio.cfg = c; }).catch(function () { audio.loading = false; });
			}
			return;
		}
		var f = audio.cfg[name];
		if (!f || !f.length || !window.RVIPSound) return;
		audio.played++;
		RVIPSound.play([f], 0.6);   /* one of the event's variants at random */
	}

	/* ---------- saves (IDBFS at RvipApp.dir = $HOME of the game) ---------- */
	function hasSave() { try { Module.FS.stat(SAVE); return true; } catch (e) { return false; } }
	app = RvipApp({
		name: 'slimy',
		save: function () { return hasSave() ? SAVE : null; },
		clear: function () { if (hasSave()) Module.FS.unlink(SAVE); },
		put: function (file, data) { Module.FS.writeFile(SAVE, data); },
		exportName: function () { return 'TSL-SAVE'; },
		noSave: 'No saved game: the web version autosaves at the start and on every level change (and S saves and quits); a finished run removes it.',
		helpText: 'Press ? in the game for its key reference.'
	});
	setInterval(function () { if (app.running) app.sync(); }, 15000);
	document.addEventListener('visibilitychange', function () { if (document.hidden && app.running) app.sync(); });
	window.addEventListener('pagehide', function () { if (app.running) app.sync(); });
	window.addEventListener('beforeunload', function (e) { if (app.running) { e.preventDefault(); e.returnValue = ''; } });

	/* the player's name: asked once (first game start), kept in /slimy/web-name,
	 * used as the hero's name (player.c) and in the graveyard report; blank = none */
	function playerName() {
		var NF = DIR + '/web-name', name = '';
		try { name = Module.FS.readFile(NF, { encoding: 'utf8' }).trim(); } catch (e) {
			var a = null; try { a = window.prompt('Your hero\'s name (also shown on the graveyard; optional):', ''); } catch (e3) { return ''; }   /* no dialogs (embedded pane): blank, ask next time */
			name = (a || '').replace(/[^\x20-\x7e]/g, '').trim().slice(0, 29);
			try { Module.FS.writeFile(NF, name); app.sync(); } catch (e2) {}   /* asked once; blank = no name */
		}
		return name;
	}

	var panes = ['map', 'stat', 'msg', 'screen'];
	window.slimyShadow = {};
	window.Module = {
		preRun: [function () {
			var FS = Module.FS;
			Module.addRunDependency('idbfs');
			RvipApp.mount(function (err) {
				if (err) status('Could not read saved games from IndexedDB (' + err + ').', true);
				Module.ENV.HOME = DIR;          /* TSL-SAVE and .tsl_conf (get_file_path) */
				FS.chdir(DIR);
				loadLayout();                   /* before the game: the tile set by name */
				renderTiles();
				$('game').hidden = false; makeWM(); loadFace(L.face); if (L.mapFace) loadFace(L.mapFace);
				$('sel-font').value = L.face || '';
				if (L.tiles !== 'None') loadSheets(function () { Module.removeRunDependency('idbfs'); });
				else Module.removeRunDependency('idbfs');
			});
		}],
		onRuntimeInitialized: function () { app.running = true; status(''); },
		print: function (s) { console.log(s); },
		printErr: function (s) { console.warn(s); },
		setStatus: function (s) { if (s && !app.running) status(s.replace(/\(\d+\/\d+\)/, '').trim() || 'Loading…'); },
		onAbort: function (what) { app.crashed(what); },
		rvipSound: function (e) { sound(e); },
		rvipLog: function (t) { RvipWM.log($('log'), t); },   /* each game message as flushed (message.c) */
		rvipInv: function (s) {                                /* lines "letter\tname\tclass" from player.c */
			var b = $('inv');
			b.textContent = '';
			s.split('\n').forEach(function (l) {
				if (!l) return;
				var f = l.split('\t'), d = document.createElement('div'), t = document.createElement('span'), k = document.createElement('b');
				k.textContent = f[0] + ') ';
				t.appendChild(k); t.appendChild(document.createTextNode(f[1])); d.appendChild(t);
				if (f[2]) d.className = f[2];
				b.appendChild(d);
			});
			if (!b.firstChild) { var e = document.createElement('div'); e.textContent = '(nothing)'; b.appendChild(e); }
		},
		rvipVis: function (s) { lastVis = s; drawVis(); },   /* creatures + items in view, from console.c */
		rvipName: playerName,
		rvipLevel: function (i) { music.level = i; musicUpdate(); },
		rvipBeacon: function (ev, killer, depth, turns) {   /* RVIP stage 9: graveyard report, fields from the C side */
			try {
				var name = playerName();
				var p = [['g', 'slimy'], ['ev', ev], ['name', name], ['killer', killer], ['depth', depth >= 0 ? depth : ''], ['turns', turns]];
				var q = p.filter(function (a) { return a[1] !== ''; })
					.map(function (a) { return a[0] + '=' + encodeURIComponent(a[1]); }).join('&');
				if (window.RvipWM && RvipWM.report) RvipWM.report(q); else fetch('/roguelikes/beacon?' + q, { keepalive: true, mode: 'no-cors' }).catch(function () {});
			} catch (e) {}
		},
		rvipSync: function () { return new Promise(function (r) { app.sync(function () { r(); }); }); },
		rvipEnd: function () {                /* death or save-and-quit: persist, then a new game */
			app.running = false;
			return new Promise(function (r) {
				app.sync(function () { status('Game over. Starting a new game…'); setTimeout(function () { location.reload(); }, 1500); r(); });
			});
		},
		rvipMap: function (t, c, h, w, hy, hx, t2) {
			lastMap = { t: Int32Array.from(t), t2: Int32Array.from(t2), c: Uint32Array.from(c), h: h, w: w, hy: hy, hx: hx };
			window.slimyTiles = lastMap;
			redrawMap();
		},
		rvipPane: function (pane, html, rows) {
			var id = panes[pane], e = $(id);
			e.innerHTML = html;
			slimyShadow[id] = e.textContent;
			if (pane === 3) popShow(rows > 0);
			else if (pane === 0) popShow(false);
			if (pane === 2) e.parentNode.scrollTop = e.parentNode.scrollHeight;
		}
	};

	/* ---------- keys ---------- */
	var keyMap = { ArrowUp: 259, ArrowDown: 258, ArrowLeft: 260, ArrowRight: 261, Escape: 27, Enter: 10, Backspace: 8, Tab: 9 };
	document.addEventListener('keydown', function (e) {
		var tg = e.target && e.target.tagName;
		if (tg === 'INPUT' || tg === 'TEXTAREA' || tg === 'SELECT') return;
		if (!app.running) return;
		var k = keyMap[e.key];
		if (/^Numpad[0-9]$/.test(e.code)) k = 0x1000 + (e.code.charCodeAt(6) - 48);   /* numpad as own codes */
		if (k === undefined && e.key.length === 1) { k = e.key.charCodeAt(0); if (e.ctrlKey) k &= 0x1f; }
		if (k === undefined || !Module._web_key) return;
		if (e.metaKey || e.altKey) return;
		e.preventDefault();
		Module._web_key(k);
	});

	document.addEventListener('DOMContentLoaded', function () {
		$('btn-tiles').onclick = toggleTiles;
		RvipWM.dropdown($('btn-audio'), $('menu-audio'));
		RvipWM.dropdown($('btn-file'), $('menu-file'));
		$('chk-sound').onchange = function () { if (!L) return; L.audio.sound = this.checked; if (this.checked) sound(''); saveLayout(); };
		$('chk-music').onchange = function () { if (!L) return; L.audio.music = this.checked; saveLayout(); musicUpdate(); };
		RvipWM.fonts.then(function () {
			[[$('sel-font'), 'face'], [mapSel, 'mapFace']].forEach(function (a) { RvipWM.fontOptions(a[0]); a[0].value = (L && L[a[1]]) || ''; });
		}).catch(function () { });
		[[$('sel-font'), 'face'], [mapSel, 'mapFace']].forEach(function (a) {
			a[0].onchange = function () { if (!L) return; L[a[1]] = this.value; saveLayout(); loadFace(this.value); this.blur(); };
		});
		document.querySelectorAll('button').forEach(function (b) { b.addEventListener('mousedown', function (e) { e.preventDefault(); }); });
	});
	/* the checkbox reflects the saved choice; if saved on, load sounds.json now */
	var chk = setInterval(function () { if (L) { $('chk-sound').checked = L.audio.sound; $('chk-music').checked = L.audio.music; if (L.audio.sound) sound(''); musicUpdate(); clearInterval(chk); } }, 100);
})();
