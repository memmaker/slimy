/*
  RVIP web port: curses subset for TSL's console frontend (console.c).
  Each curses window is routed by identity to a page pane:
  board_win -> map, status_win -> status, message_bar -> messages,
  stdscr -> screen (full-screen texts, menus). wrefresh sends the
  window's rows as HTML lines (colours/attrs decided here, rule 7).
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "curses.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern WINDOW * board_win;
extern WINDOW * status_win;
extern WINDOW * message_bar;

WINDOW * stdscr = NULL;
int LINES = 24, COLS = 80;

enum { PANE_MAP, PANE_STATUS, PANE_MSG, PANE_SCREEN };

static WINDOW * mkwin(int h, int w)
{
  WINDOW * win = calloc(1, sizeof(WINDOW));
  win->h = h; win->w = w;
  win->cells = malloc(sizeof(chtype) * h * w);
  werase(win);
  return win;
}

WINDOW * initscr(void)
{
  if (stdscr == NULL) stdscr = mkwin(LINES, COLS);
  stdscr->pane = PANE_SCREEN;
  return stdscr;
}

int endwin(void) { return OK; }

WINDOW * newwin(int h, int w, int y, int x)
{
  (void)y; (void)x;
  if (h <= 0) h = LINES - y;
  if (w <= 0) w = COLS - x;
  return mkwin(h, w);
}

int delwin(WINDOW * w)
{
  if (w == NULL) return ERR;
  free(w->cells);
  free(w);
  return OK;
}

int cbreak(void) { return OK; }
int noecho(void) { return OK; }
int curs_set(int v) { (void)v; return OK; }
int keypad(WINDOW * w, int b) { (void)w; (void)b; return OK; }
int start_color(void) { return OK; }
int init_pair(short p, short f, short b) { (void)p; (void)f; (void)b; return OK; }

int wmove(WINDOW * w, int y, int x)
{
  if (y < 0 || x < 0 || y >= w->h || x >= w->w) return ERR;
  w->cy = y; w->cx = x;
  return OK;
}

int waddch(WINDOW * w, chtype c)
{
  chtype ch = c & A_CHARTEXT;

  if (ch == '\n')
  {
    int x;
    for (x = w->cx; x < w->w; x++) w->cells[w->cy * w->w + x] = ' ';
    w->cx = 0;
    if (w->cy < w->h - 1) w->cy++;
    return OK;
  }
  if (w->cy >= w->h) return ERR;
  w->cells[w->cy * w->w + w->cx] = ch | (c & ~A_CHARTEXT) | w->attr;
  if (++w->cx >= w->w)
  {
    w->cx = 0;
    if (w->cy < w->h - 1) w->cy++;
  }
  return OK;
}

int waddstr(WINDOW * w, const char * s)
{
  while (*s) waddch(w, (unsigned char)*s++);
  return OK;
}

int wprintw(WINDOW * w, const char * fmt, ...)
{
  char buf[2048];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  return waddstr(w, buf);
}

int werase(WINDOW * w)
{
  int i;
  for (i = 0; i < w->h * w->w; i++) w->cells[i] = ' ';
  w->cy = w->cx = 0;
  return OK;
}

int wattron(WINDOW * w, unsigned int a) { w->attr |= a; return OK; }
int wattroff(WINDOW * w, unsigned int a) { w->attr &= ~a; return OK; }
int wattrset(WINDOW * w, unsigned int a) { w->attr = a; return OK; }

static const char * acs_utf8(unsigned int ch)
{
  switch (ch)
  {
    case ACS_BLOCK:    return "\xe2\x96\x88";
    case ACS_CKBOARD:  return "\xc2\xb7";
    case ACS_VLINE:    return "\xe2\x94\x82";
    case ACS_HLINE:    return "\xe2\x94\x80";
    case ACS_ULCORNER: return "\xe2\x94\x8c";
    case ACS_URCORNER: return "\xe2\x94\x90";
    case ACS_LLCORNER: return "\xe2\x94\x94";
    case ACS_LRCORNER: return "\xe2\x94\x98";
    case ACS_LTEE:     return "\xe2\x94\x9c";
    case ACS_RTEE:     return "\xe2\x94\xa4";
    case ACS_BTEE:     return "\xe2\x94\xb4";
    case ACS_TTEE:     return "\xe2\x94\xac";
    case ACS_PLUS:     return "\xe2\x94\xbc";
  }
  return "?";
}

#ifdef __EMSCRIPTEN__
EM_JS(void, js_pane, (int pane, const char * html, int rows), {
  if (Module.rvipPane) Module.rvipPane(pane, UTF8ToString(html), rows);
});
#else
static void js_pane(int pane, const char * html, int rows)
{
  (void)pane; (void)html; (void)rows;
}
#endif

/* RVIP tiles: per map cell the tile code map_put chose and the cell it
   wrote; a tile is sent only while the cell still holds that char+attr,
   so anything else drawn on the board (targeting, overlays) stays text. */
#define TMAX (64 * 160)
static int tile_code[TMAX];
static chtype tile_cell[TMAX];
static int tile_out[TMAX];

void web_map_tile(int y, int x, int code)
{
  int i;
  if (board_win == NULL || y < 0 || x < 0 || y >= board_win->h || x >= board_win->w) return;
  i = y * board_win->w + x;
  if (i >= TMAX) return;
  tile_code[i] = code;
  tile_cell[i] = board_win->cells[i];
}

static int hero_y = -1, hero_x = -1;
void web_map_hero(int y, int x) { hero_y = y; hero_x = x; }

#ifdef __EMSCRIPTEN__
EM_JS(void, js_map, (const int * tiles, const unsigned int * cells, int h, int w, int hy, int hx), {
  if (Module.rvipMap) Module.rvipMap(new Int32Array(HEAPU8.buffer, tiles, h * w),
                                     new Uint32Array(HEAPU8.buffer, cells, h * w), h, w, hy, hx);
});
EM_ASYNC_JS(void, web_sync, (void), {
  if (Module.rvipSync) await Module.rvipSync();
});
EM_ASYNC_JS(void, web_end_js, (void), {
  if (Module.rvipEnd) await Module.rvipEnd();
  await new Promise(function () {});   /* the page reloads */
});
void web_end(void) { web_end_js(); }
/* RVIP stage 9: graveyard beacon; the page adds the name and sends it through RvipWM.report */
EM_JS(void, web_beacon, (const char * ev, const char * killer, int depth, long turns), {
  try { if (Module.rvipBeacon) Module.rvipBeacon(UTF8ToString(ev), UTF8ToString(killer), depth, Number(turns)); } catch (e) {}
});
/* RVIP: sound event from a game action (RVIP_SOUND, main.h); the page plays it */
EM_JS(void, web_sound, (const char * e), {
  if (Module.rvipSound) Module.rvipSound(UTF8ToString(e));
});
#else
static void js_map(const int * t, const unsigned int * c, int h, int w, int hy, int hx)
{ (void)t; (void)c; (void)h; (void)w; (void)hy; (void)hx; }
void web_sync(void) { }
void web_end(void) { }
void web_beacon(const char * ev, const char * killer, int depth, long turns) { (void)ev; (void)killer; (void)depth; (void)turns; }
#endif

static void send_map_tiles(WINDOW * w)
{
  int i, n = w->h * w->w;
  if (n > TMAX) n = TMAX;
  for (i = 0; i < n; i++)
    tile_out[i] = (tile_cell[i] == w->cells[i] && (w->cells[i] & A_CHARTEXT) != ' ') ? tile_code[i] : -1;
  js_map(tile_out, w->cells, w->h, w->w, hero_y, hero_x);
}

/* Builds the window as HTML lines, trimmed (rule 5), and sends it. */
int wrefresh(WINDOW * w)
{
  static char * out = NULL;
  static size_t cap = 0;
  size_t len = 0;
  int y, x, rows = 0, pane;

  if (w == board_win) pane = PANE_MAP;
  else if (w == status_win) pane = PANE_STATUS;
  else if (w == message_bar) pane = PANE_MSG;
  else pane = PANE_SCREEN;

  if (cap < (size_t)(w->h * w->w * 48 + 64))
  {
    cap = w->h * w->w * 48 + 64;
    out = realloc(out, cap);
  }

  for (y = 0; y < w->h; y++)
  {
    int last = -1;
    unsigned int cur = 0;
    for (x = 0; x < w->w; x++)
    {
      chtype c = w->cells[y * w->w + x];
      if ((c & A_CHARTEXT) != ' ' || (c & (A_REVERSE))) last = x;
    }
    if (last >= 0) rows = y + 1;
    for (x = 0; x <= last; x++)
    {
      chtype c = w->cells[y * w->w + x];
      unsigned int a = c & ~A_CHARTEXT;
      unsigned int ch = c & A_CHARTEXT;
      if (a != cur)
      {
        if (cur) len += sprintf(out + len, "</span>");
        if (a)
          len += sprintf(out + len, "<span class=\"c%u%s%s%s\">", PAIR_NUMBER(a),
                         (a & A_REVERSE) ? " rv" : "", (a & A_BOLD) ? " bd" : "",
                         (a & A_DIM) ? " dm" : "");
        cur = a;
      }
      if (ch >= 0xE000) len += sprintf(out + len, "%s", acs_utf8(ch));
      else if (ch == '<') len += sprintf(out + len, "&lt;");
      else if (ch == '>') len += sprintf(out + len, "&gt;");
      else if (ch == '&') len += sprintf(out + len, "&amp;");
      else out[len++] = (ch >= 32 && ch < 127) ? (char)ch : '?';
    }
    if (cur) len += sprintf(out + len, "</span>");
    out[len++] = '\n';
  }
  /* drop trailing empty lines (rule 5) */
  while (len > 0 && out[len - 1] == '\n') len--;
  out[len] = 0;
  js_pane(pane, out, rows);
  if (pane == PANE_MAP) send_map_tiles(w);
  return OK;
}

#ifdef __EMSCRIPTEN__
#define KQ 256
static int kq[KQ];
static int kq_head = 0, kq_tail = 0;

EMSCRIPTEN_KEEPALIVE void web_key(int k)
{
  int n = (kq_tail + 1) % KQ;
  if (n == kq_head) return;
  kq[kq_tail] = k;
  kq_tail = n;
}

int getch(void)
{
  int k;
  while (kq_head == kq_tail) emscripten_sleep(10);
  k = kq[kq_head];
  kq_head = (kq_head + 1) % KQ;
  return k;
}

/* RVIP explore: take a pending key without blocking (-1: none). */
int web_poll_key(void)
{
  int k;
  if (kq_head == kq_tail) return -1;
  k = kq[kq_head];
  kq_head = (kq_head + 1) % KQ;
  return k;
}

void web_pause(int ms) { emscripten_sleep(ms); }
#else
int getch(void) { int c = getchar(); return c == EOF ? 27 : c; }
int web_poll_key(void) { return -1; }
void web_pause(int ms) { (void)ms; }
#endif
