/*
  RVIP web port: minimal curses subset for The Slimy Lichmummy.
  Only what console.c/glyph.c/debug.c use; implemented in port/wcurses.c.
  console.c (the game's curses frontend) is compiled unchanged via -Iport.
*/
#ifndef RVIP_CURSES_H
#define RVIP_CURSES_H

#include <stdarg.h>
#include <stdio.h>  /* real curses.h pulls it in; console.c relies on it */

typedef unsigned int chtype;

typedef struct rvip_window
{
  int pane;              /* 0 map, 1 status, 2 messages, 3 screen */
  int h, w;
  int cy, cx;
  unsigned int attr;
  chtype * cells;        /* char | attr */
  int touched;           /* written since the last wrefresh */
} WINDOW;

extern WINDOW * stdscr;
extern int LINES, COLS;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define ERR (-1)
#define OK 0

/* attributes: char in low 16 bits, attrs above */
#define A_CHARTEXT 0x0000ffffu
#define A_COLOR    0x000f0000u
#define COLOR_PAIR(n) ((((unsigned int)(n)) & 0xf) << 16)
#define PAIR_NUMBER(a) (((a) & A_COLOR) >> 16)
#define A_NORMAL   0u
#define A_REVERSE  0x00100000u
#define A_BOLD     0x00200000u
#define A_DIM      0x00400000u
#define A_UNDERLINE 0x00800000u
#define A_STANDOUT A_REVERSE

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

/* line drawing: private code points, turned into UTF-8 on output */
#define ACS_BLOCK    0xE000
#define ACS_CKBOARD  0xE001
#define ACS_VLINE    0xE002
#define ACS_HLINE    0xE003
#define ACS_ULCORNER 0xE004
#define ACS_URCORNER 0xE005
#define ACS_LLCORNER 0xE006
#define ACS_LRCORNER 0xE007
#define ACS_LTEE     0xE008
#define ACS_RTEE     0xE009
#define ACS_BTEE     0xE00A
#define ACS_TTEE     0xE00B
#define ACS_PLUS     0xE00C

#define KEY_BACKSPACE 263

void web_map_tile(int y, int x, int code); /* RVIP tiles, map pane */
void web_map_tile2(int y, int x, int code); /* second set (tsl-go), stored before web_map_tile */
void web_map_hero(int y, int x); /* RVIP camera: hero's board cell */
void web_sync(void); /* RVIP: persist IDBFS (awaits) */
void web_end(void); /* RVIP: game over: sync, page reloads */

WINDOW * initscr(void);
int endwin(void);
WINDOW * newwin(int h, int w, int y, int x);
int delwin(WINDOW * w);
int cbreak(void);
int noecho(void);
int curs_set(int v);
int keypad(WINDOW * w, int b);
int start_color(void);
int init_pair(short p, short f, short b);

int wmove(WINDOW * w, int y, int x);
int waddch(WINDOW * w, chtype c);
int waddstr(WINDOW * w, const char * s);
int wprintw(WINDOW * w, const char * fmt, ...);
int werase(WINDOW * w);
int wrefresh(WINDOW * w);
int wattron(WINDOW * w, unsigned int a);
int wattroff(WINDOW * w, unsigned int a);
int wattrset(WINDOW * w, unsigned int a);
int getch(void);

#define getyx(w, y, x) ((y) = (w)->cy, (x) = (w)->cx)
#define erase() werase(stdscr)
#define clear() werase(stdscr)
#define refresh() wrefresh(stdscr)
#define addch(c) waddch(stdscr, (c))
#define addstr(s) waddstr(stdscr, (s))
#define attron(a) wattron(stdscr, (a))
#define attroff(a) wattroff(stdscr, (a))
#define printw(...) wprintw(stdscr, __VA_ARGS__)

#endif
