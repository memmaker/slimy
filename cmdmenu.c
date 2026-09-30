/*
  RVIP: floating command menu (Enter). Lists every command, grouped as
  the key reference in help.c groups them, with the key bound in the
  current keymap. No movement entries. Choosing returns the key (or the
  action when nothing is bound) to player_control(), so every command
  keeps its own prompts.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "ui.h"
#include "input.h"
#include "keymap.h"
#include "cmdmenu.h"

#ifdef TSL_CONSOLE
#include <curses.h>
#define CMD_SCREEN_ROWS LINES
#else
#define CMD_SCREEN_ROWS 25
#endif

int inv_reopen = 0;

typedef struct
{
  const char * group; /* non-NULL: a group header row */
  action_t action;
  int key;            /* fixed key (0: first key bound to ACTION) */
  const char * label;
} cmd_entry_t;

static const cmd_entry_t cmd_table[] =
{
  { "Actions", 0, 0, NULL },
  { NULL, action_wait,      0,   "wait (pass) one turn" },
  { NULL, action_rest,      0,   "rest" },
  { NULL, action_explore,   0,   "auto-explore" },
  { NULL, action_stairs,    0,   "traverse stairs" },
  { NULL, action_stairs,    '<', "walk to stairs up" },
  { NULL, action_stairs,    '>', "walk to stairs down" },
  { NULL, action_close,     0,   "close adjacent door" },
  { NULL, action_interact,  0,   "interact with environment" },
  { NULL, action_inspect,   0,   "inspect a tile" },
  { NULL, action_select,    0,   "do what I mean (here)" },
  { "Items", 0, 0, NULL },
  { NULL, action_inventory, 0,   "browse inventory" },
  { NULL, action_use,       0,   "use item" },
  { NULL, action_apply,     0,   "apply item" },
  { NULL, action_pickup,    0,   "pick up" },
  { NULL, action_drop,      0,   "drop" },
  { NULL, action_equip,     0,   "equip" },
  { NULL, action_remove,    0,   "remove" },
  { NULL, action_drink,     0,   "drink" },
  { NULL, action_eat,       0,   "eat" },
  { NULL, action_read,      0,   "read" },
  { NULL, action_label,     0,   "label (for taking notes)" },
  { "Missiles", 0, 0, NULL },
  { NULL, action_fire,      0,   "fire missile" },
  { NULL, action_throw,     0,   "throw an item" },
  { NULL, action_quiver,    0,   "cycle ammo" },
  { "Abilities", 0, 0, NULL },
  { NULL, action_ability,   0,   "use ability (spell, skill)" },
  { NULL, action_ability_config, 0, "assign ability shortcuts" },
  { "Game", 0, 0, NULL },
  { NULL, action_help,      0,   "help" },
  { NULL, action_history,   0,   "message history" },
  { NULL, action_status,    0,   "character summary" },
  { NULL, action_flip,      0,   "flip status display" },
  { NULL, action_recenter,  0,   "recenter view" },
  { NULL, action_redraw,    0,   "redraw the screen" },
  { NULL, action_options,   0,   "change options" },
  { NULL, action_version,   0,   "display version" },
  { NULL, action_save,      0,   "save and quit" },
  { NULL, action_quit,      0,   "forfeit game" },
};

#define CMD_ROWS ((int)(sizeof(cmd_table) / sizeof(cmd_table[0])))
#define CMD_TITLE "Commands"


/* The key an entry runs: fixed, else a printable key bound to it, else any. */
static int cmd_key(const cmd_entry_t * e)
{
  int b;
  int any = 0;

  if (e->key)
    return e->key;

  for (b = 0; b < MAX_BINDINGS; b++)
  {
    int k = keymap[e->action][b];

    if (k == 0)
      continue;

    /* Enter opens this menu; show the other key. */
    if (k == '\n' || k == '\r')
      continue;

    if (k > ' ' && k < 127)
      return k;

    if (any == 0)
      any = k;
  }

  return any;
}


static void cmd_label(char * dest, const cmd_entry_t * e)
{
  int k = cmd_key(e);

  if (k == 0)
    strcpy(dest, "");
  else
    key_to_label(dest, false, k);
}


/*
  Shows the menu. Returns the key to run (0 when it is unbound; then
  *ACTION holds the action), or -1 when cancelled.
*/
int command_menu(action_t * action)
{
  char keys[CMD_ROWS][20];
  char line[100];
  int keyw = 0;
  int w;
  int h;
  int view;
  int top = 0;
  int cur;
  int i;
  int r;
  int in;

  for (i = 0; i < CMD_ROWS; i++)
  {
    keys[i][0] = '\0';

    if (cmd_table[i].group == NULL)
    {
      cmd_label(keys[i], &cmd_table[i]);

      if ((int)strlen(keys[i]) > keyw)
	keyw = strlen(keys[i]);
    }
  }

  /* Width: longest entry (key column + space + label) or the title. */
  w = strlen(CMD_TITLE);

  for (i = 0; i < CMD_ROWS; i++)
  {
    int len;

    if (cmd_table[i].group)
      len = strlen(cmd_table[i].group) + 2;
    else
      len = keyw + 1 + strlen(cmd_table[i].label);

    if (len > w)
      w = len;
  }

  view = CMD_ROWS;

  if (view > CMD_SCREEN_ROWS - 2)
    view = CMD_SCREEN_ROWS - 2;

  h = view + 2;

  cur = 1;

  while (1)
  {
    if (cur < top)
      top = cur;

    if (cur >= top + view)
      top = cur - view + 1;

    /* Show a group header above the first entry of a scrolled view. */
    if (top > 0 && top == cur && cmd_table[top - 1].group)
      top--;

    scr_erase();

    /* Border with the title in the top line. */
    scr_move(0, 0);
    scr_special(ST_SE); /* TSL corner names: the corner's opening side */

    for (i = 0; i < w; i++)
      scr_special(ST_HLINE);

    scr_special(ST_SW);
    scr_move(0, 1);
    scr_addstr(CMD_TITLE);

    for (r = 0; r < view; r++)
    {
      i = top + r;

      scr_move(r + 1, 0);
      scr_special(ST_VLINE);
      scr_move(r + 1, w + 1);
      scr_special(ST_VLINE);

      scr_move(r + 1, 1);

      if (cmd_table[i].group)
      {
	sprintf(line, "(%s)", cmd_table[i].group);
	scr_addstr(line);
	continue;
      }

      sprintf(line, "%*s %-*s", keyw, keys[i],
	      (int)(w - keyw - 1), cmd_table[i].label);

      if (i == cur)
	scr_rev(true);

      scr_addstr(line);

      if (i == cur)
	scr_rev(false);
    }

    scr_move(h - 1, 0);
    scr_special(ST_NE);

    for (i = 0; i < w; i++)
      scr_special(ST_HLINE);

    scr_special(ST_NW);

    if (top > 0)
    {
      scr_move(0, w);
      scr_addch('^');
    }

    if (top + view < CMD_ROWS)
    {
      scr_move(h - 1, w);
      scr_addch('v');
    }

    scr_flush();

    in = get_keypress();

    if (in == kt_escape || in == kt_np0 || in == ' ' || in == '.' ||
	in == '0')
    {
      r = -1;
      break;
    }

    if (in == kt_dir_up || in == kt_np8 || in == '8')
    {
      do
	cur = (cur + CMD_ROWS - 1) % CMD_ROWS;
      while (cmd_table[cur].group);
      continue;
    }

    if (in == kt_dir_down || in == kt_np2 || in == '2')
    {
      do
	cur = (cur + 1) % CMD_ROWS;
      while (cmd_table[cur].group);
      continue;
    }

    if (in == '\n' || in == '\r' || in == kt_np5 || in == '5')
    {
      *action = cmd_table[cur].action;
      r = cmd_key(&cmd_table[cur]);
      break;
    }

    /* A command's own key chooses it. */
    for (i = 0; i < CMD_ROWS; i++)
    {
      if (cmd_table[i].group == NULL && in == cmd_key(&cmd_table[i]))
	break;
    }

    if (i < CMD_ROWS)
    {
      *action = cmd_table[i].action;
      r = in;
      break;
    }
  }

  scr_erase();
  scr_flush();

  return r;
} /* command_menu */
