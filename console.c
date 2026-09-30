#include <stdlib.h>

#include <curses.h>

#include "console.h"
#include "ui.h"
#include "glyph.h"
#include "game.h"
#ifdef __EMSCRIPTEN__
#include "places.h"
#include "tslgo_tiles.h"
#ifdef __EMSCRIPTEN__
#include <string.h>
#include "stuff.h"
#include "fov.h"
void web_visible(const char * t);
#endif
#endif


const unsigned int gent_width = 1;


/*
  Starts ncurses.
*/
void init_ui()
{
  init_messages();

  board_size_y = 20;
  board_size_x = 40;

  scroll_limit_y = 3;
  scroll_limit_x = 5;

  message_bar = NULL;
  board_win = NULL;
  status_win = NULL;

  initscr();

  if ((LINES < 24) || (COLS < 80))
  {
    shutdown_everything();
    printf("Terminal too small.");
    exit(1);
  }

  init_glyph_map();
  
  /* We have the terminal we need, now try to set up the windows. */
  message_bar  = newwin(MESSAGE_LINES, board_size_x, board_size_y, 0);
  board_win    = newwin(board_size_y, board_size_x, 0, 0);
  status_win   = newwin(24, 0, 0, board_size_x);

  /* Did we get all of them? */
  if ((message_bar == NULL) ||
      (board_win == NULL) ||
      (status_win == NULL))
  {
    shutdown_everything();
    printf("Couldn't create ncurses windows.");
    exit(2);
  }
  
  /*
    Set up some other things. We wish to catch control codes, disable
    echoing and hide the cursor.
  */
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);

  start_color();
  init_pair(color_black,   COLOR_BLACK,   COLOR_BLACK);
  init_pair(color_brown,   COLOR_YELLOW,  COLOR_BLACK);
  init_pair(color_blue,    COLOR_BLUE,    COLOR_BLACK);
  init_pair(color_red,     COLOR_RED,     COLOR_BLACK);
  init_pair(color_green,   COLOR_GREEN,   COLOR_BLACK);
  init_pair(color_cyan,    COLOR_CYAN,    COLOR_BLACK);
  init_pair(color_magenta, COLOR_MAGENTA, COLOR_BLACK);

  /* The UI is running, congratulations */
  clear();
  refresh();
  
  return;
} /* init_ui */



/*
  Shuts down ncurses.
*/
void shutdown_ui()
{
  clear_messages();

  if (message_bar != NULL) delwin(message_bar);
  if (board_win != NULL) delwin(board_win);
  if (status_win != NULL) delwin(status_win);

  message_bar = NULL;
  board_win = NULL;
  status_win = NULL;

  endwin();

  return;
} /* shutdown_ui */


/*
  Waits for the user to press a key and returns the keypress.
*/
key_token_t get_keypress()
{
  int inp;

  inp = getch();

#ifdef __EMSCRIPTEN__
  /* RVIP: the web page sends numpad digits as 0x1000 + digit. */
  if (inp >= 0x1000 && inp <= 0x1009)
    return kt_np0 + (inp - 0x1000);
#endif

  switch (inp)
  {
    case 27:   return kt_escape;

/*    case KEY_NPAGE: return kt_page_down;
      case KEY_PPAGE: return kt_page_up;*/

#ifndef _WIN32
    case 360: return kt_np1;
    case 258: return kt_dir_down;
    case 338: return kt_np3;
    case 260: return kt_dir_left;
    case 350: return kt_np5;
    case 261: return kt_dir_right;
    case 262: return kt_np7;
    case 259: return kt_dir_up;
    case 339: return kt_np9;
      
      /*
	case 258: return kt_np2;
	case 260: return kt_np4;
	case 261: return kt_np6;
	case 259: return kt_np8;
      */
#else
    case 455: return kt_np1;
    case 456: return kt_np2;
    case 457: return kt_np3;
    case 452: return kt_np4;
      /*case : return kt_np5;*/
    case 454: return kt_np6;
    case 449: return kt_np7;
    case 450: return kt_np8;
    case 451: return kt_np9;
#endif

      /*
	We need to check for in == 8 (ASCII for backspace) to make it
	work under PDCurses/Windows.
      */
    case KEY_BACKSPACE:
    case 8:
      return kt_backspace;

    default:
      return inp;
  }
}


void game_over(const char * msg, const blean_t won)
{
  erase();
  printw("%s\n(Press Enter to exit)\n", msg);
  get_keypress();

  return;
} /* game_over */



void st_special(const unsigned int c)
{
  chtype t;

  switch (c)
  {
    case ST_HLINE: t = ACS_HLINE; break;
    case ST_VLINE: t = ACS_VLINE; break;
    case ST_RTEE:  t = ACS_RTEE; break;
    case ST_LTEE:  t = ACS_LTEE; break;
    case ST_TTEE:  t = ACS_TTEE; break;
    case ST_BTEE:  t = ACS_BTEE; break;
    case ST_CROSS: t = ACS_PLUS; break;
    case ST_SE:    t = ACS_ULCORNER; break;
    case ST_SW:    t = ACS_URCORNER; break;
    case ST_NE:    t = ACS_LLCORNER; break;
    case ST_NW:    t = ACS_LRCORNER; break;
    case ST_CURSOR:t = ACS_BLOCK; break;
    default: t = '?'; break;
  }

  waddch(status_win, t);

  return;
}



void st_erase(void)
{
  werase(status_win);
  return;
}

void st_flush(void)
{
  wrefresh(status_win);
  return;
}

void st_addch(const char c)
{
  waddch(status_win, c);
  return;
}

void st_addstr(const char * s)
{
  waddstr(status_win, s);
  return;
}

void st_move(const unsigned int y, const unsigned int x)
{
  wmove(status_win, y, x);
  return;
}

void st_rev(const blean_t new_state)
{
  st_reversed = new_state;

  if (new_state)
    wattron(status_win, A_REVERSE);
  else
    wattroff(status_win, A_REVERSE);

  return;
}

void st_dim(const blean_t new_state)
{
  if (new_state)
    wattron(status_win, A_DIM);
  else
    wattroff(status_win, A_DIM);

  return;
}

void st_getyx(unsigned int * y, unsigned int * x)
{
  unsigned int ly;
  unsigned int lx;
  
  getyx(status_win, ly, lx);

  *y = ly;
  *x = lx;

  return;
}

void map_erase(void)
{
  werase(board_win);
  return;
}

void map_cursor(const blean_t new_state)
{
  if (new_state == true)
    curs_set(1);
  else
    curs_set(0);

  return;
}

void map_flush(void)
{
  wrefresh(board_win);
  return;
}

void map_move(const unsigned int y, const unsigned int x)
{
  wmove(board_win, y, x);
}



#ifdef __EMSCRIPTEN__
static gent_t web_top_gent(const gent_t gent);

/* RVIP: a gent's icon for the Inventory/Visible windows: own-set gent
   (stand-ins applied) | tsl-go sprite << 10 (255 = none); its text glyph */
unsigned int web_gent_tile(gent_t g)
{
  unsigned int t2 = tslgo_gent[g] >= 253 ? 255 : tslgo_gent[g];
  return (unsigned int)web_top_gent(g) | (t2 << 10);
}

char web_gent_glyph(gent_t g)
{
  unsigned int ch = glyph_map[g] & A_CHARTEXT;
  return (ch > 32 && ch < 127) ? (char)ch : '?';
}

static void web_visible_line(char * buf, size_t * len, size_t cap, char kind, gent_t g, const char * name)
{
  if (*len + strlen(name) + 32 < cap)
    *len += sprintf(buf + *len, "%c%c%s\t\t%u\n", kind, web_gent_glyph(g), name, web_gent_tile(g));
}
#endif

void map_put(const unsigned int y, const unsigned int x, const gent_t gent, const unsigned char attr)
{
  unsigned int curses_attr;

  curses_attr = glyph_attr[gent];

  if (attr & MAP_REVERSE)
    curses_attr |= A_REVERSE;

  if (attr & (MAP_DIM | MAP_SLEEP) && (curses_attr & A_REVERSE) == 0)
  {
    curses_attr |= COLOR_PAIR(color_black) | A_BOLD;
  }

  put_custom(board_win, y, x, glyph_map[gent], curses_attr);

#ifdef __EMSCRIPTEN__
  /* RVIP tiles (rule 7): C picks the sprite as allui.c does: the gent's
     tile over the level's floor tile; dim/sleep -> tiledim, reverse -> tilerev. */
  {
    gent_t tg = web_top_gent(gent);
    unsigned int under = gent_floor + game->player->location->floor_type;
    unsigned int top;
    unsigned int sheet = (attr & MAP_REVERSE) ? 2 : ((attr & (MAP_DIM | MAP_SLEEP)) ? 1 : 0);
    top = (tg == gent_floor) ? under : (unsigned int)tg;
    {
      /* second set: tsl-go's sprites (port/tslgo_tiles.h, web/mktslgo.py);
         walls/floors from the level's tsl-go theme, variant by level cell */
      const level_t * lv = game->player->location;
      unsigned int li = lv->level_index < LEVELS ? lv->level_index : LEVEL_DUNGEON;
      unsigned int ly = y + view_top, lx = x + view_left, h = ly * 7 + lx * 13 + (ly ^ lx);
      unsigned int wn = 0, fn = 0, t2, u2;
      while (wn < 10 && tslgo_wall[li][wn] != 255) wn++;
      while (fn < 10 && tslgo_floor[li][fn] != 255) fn++;
      u2 = tslgo_floor[li][h % fn];
      t2 = (gent == gent_blank) ? 255 : tslgo_gent[gent];
      if (gent == gent_blank) u2 = 255;
      if (t2 == 254) { t2 = tslgo_wall[li][h % wn]; u2 = t2; }
      else if (t2 == 253) t2 = 255;
      web_map_tile2(y, x, (int)(t2 | (u2 << 8) | ((sheet == 1) << 16)));
    }
    web_map_tile(y, x, (int)(top | (under << 10) | (sheet << 20)));
  }
#endif

  return;
}



void mb_erase(void)
{
  werase(message_bar);
  return;
}

void mb_flush(void)
{
  wrefresh(message_bar);
  return;
}

void mb_move(const unsigned int y, const unsigned int x)
{
  wmove(message_bar, y, x);
  return;
}

void mb_addstr(const char * s)
{
  waddstr(message_bar, s);
  return;
}

void mb_more(void)
{
  mb_move(MESSAGE_LINES - 1, MESSAGE_BAR_WIDTH + 1 - 6);

  wattron(message_bar, A_REVERSE);
  waddstr(message_bar, MORE_MARKER);
  wattrset(message_bar, 0);
 
  mb_flush();
}

void mb_cursor(blean_t new_state)
{
  if (new_state == true)
    curs_set(1);
  else
    curs_set(0);

  return;
}


void scr_erase(void)
{
  erase();
}


void scr_move(const unsigned int y, const unsigned int x)
{
  wmove(stdscr, y, x);
}


void scr_getyx(unsigned int * y, unsigned int * x)
{
  unsigned int ly;
  unsigned int lx;
  
  getyx(stdscr, ly, lx);

  *y = ly;
  *x = lx;

  return;
}


void scr_addgent(const gent_t gent, const unsigned int attr)
{
  unsigned int y;
  unsigned int x;
  unsigned int curses_attr;

  curses_attr = glyph_attr[gent];

  if (attr & MAP_REVERSE)
    curses_attr |= A_REVERSE;

  if (attr & MAP_DIM)
    curses_attr |= A_DIM;

  scr_getyx(&y, &x);

  put_custom(stdscr, y, x, glyph_map[gent], curses_attr);

  wmove(status_win, y, x + 1);
}


void scr_addch(const char c)
{
  addch(c);
}


void scr_addstr(const char * s)
{
  addstr(s);
}


void scr_flush(void)
{
  refresh();
}



void st_gent(const gent_t gent)
{
  unsigned int y;
  unsigned int x;

  st_getyx(&y, &x);

  put_custom(status_win, y, x, glyph_map[gent], glyph_attr[gent] | (st_reversed ? A_REVERSE : 0));

  wmove(status_win, y, x + 1);

  return;
}



void scr_rev(const blean_t new_state)
{
  if (new_state)
    attron(A_REVERSE);
  else
    attroff(A_REVERSE);

  return;
}



void scr_special(const unsigned int c)
{
  chtype t;

  switch (c)
  {
    case ST_HLINE: t = ACS_HLINE; break;
    case ST_VLINE: t = ACS_VLINE; break;
    case ST_RTEE:  t = ACS_RTEE; break;
    case ST_LTEE:  t = ACS_LTEE; break;
    case ST_TTEE:  t = ACS_TTEE; break;
    case ST_BTEE:  t = ACS_BTEE; break;
    case ST_CROSS: t = ACS_PLUS; break;
    case ST_SE:    t = ACS_ULCORNER; break;
    case ST_SW:    t = ACS_URCORNER; break;
    case ST_NE:    t = ACS_LLCORNER; break;
    case ST_NW:    t = ACS_LRCORNER; break;
    case ST_CURSOR:t = ACS_BLOCK; break;
    default: t = '?'; break;
  }

  addch(t);

  return;
}

#ifdef __EMSCRIPTEN__
/* RVIP stand-ins: gents with an empty slot in tileset.png get a
   sprite from the same sheet (never text, never another set) */
static gent_t web_top_gent(const gent_t gent)
{
  static const gent_t stand_in[][2] = {
    { gent_amulet, gent_crown }, { gent_beetle_shell, gent_carcass },
    { gent_bone_dust, gent_bone }, { gent_caeltzan, gent_necromancer },
    { gent_chickpeas, gent_bread }, { gent_cranium, gent_decapitated_head },
    { gent_eyeball, gent_floating_brain }, { gent_falafel, gent_bread },
    { gent_fish, gent_carcass }, { gent_lognac, gent_goatman },
    { gent_mandrake_root, gent_mushroom }, { gent_meat, gent_carcass },
    { gent_mummy_wrapping, gent_robe }, { gent_prod, gent_staff },
    { gent_sausage, gent_cheese }, { gent_ybznek, gent_nameless_horror } };
  unsigned int k;

  for (k = 0; k < sizeof(stand_in) / sizeof(stand_in[0]); k++)
    if (stand_in[k][0] == gent) return stand_in[k][1];
  return gent;
}

/*
  RVIP: the Visible window. What draw_level() shows in view: lines
  "M<glyph><name>\t\t<tile>" per creature, "I..." per item; tile =
  own-set gent | tsl-go sprite << 10 (255 = none). Sent when changed.
*/
void web_send_visible(void)
{
  static char * last = NULL;
  static char buf[8192];
  size_t len = 0;
  level_t * level = get_current_level();
  unsigned int i;
  creature_t * c;
  item_t * item;

  buf[0] = '\0';

  for (i = 0; i < level->creatures; i++)
  {
    c = level->creature[i];

    if (c == NULL || is_player(c) || c->detected == false ||
	can_see_creature(game->player, c) == false)
      continue;

    web_visible_line(buf, &len, sizeof(buf), 'M', c->gent, c->name_one);
  }

  for (item = level->first_item; item != NULL; item = item->next_item)
  {
    char * name;

    if (on_map(level, item->y, item->x) == false ||
	get_tile(level, item->y, item->x) == tile_forcefield ||
	can_see(game->player, item->y, item->x) == false)
      continue;

    name = get_item_name(item);
    web_visible_line(buf, &len, sizeof(buf), 'I', item->gent, name);
    free(name);
  }

  if (last != NULL && strcmp(last, buf) == 0)
    return;

  free(last);
  last = mydup(buf);
  web_visible(buf);
}
#endif
