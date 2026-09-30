/*
  RVIP auto-explore and walk-to-stairs (web port addition).

  explore_start() arms a walk; player_control() asks explore_step()
  for one move action per turn before reading a key. The explorer
  only uses what the player knows: level->memory[][] (remembered
  tiles, doors, revealed traps) and what can_see() shows now.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "game.h"
#include "level.h"
#include "tiles.h"
#include "gent.h"
#include "creature.h"
#include "fov.h"
#include "item.h"
#include "message.h"
#include "input.h"
#include "ui.h"
#include "places.h"
#include "attrs.h"
#include "find.h"
#include "explore.h"

#ifdef __EMSCRIPTEN__
int web_poll_key(void);
void web_pause(int ms);
#endif

int explore_mode = EXPLORE_OFF;

/* Per-level explorer state (reset when the level changes). */
static int ex_level = -1;
static unsigned int ex_sy, ex_sx;
static unsigned char * ex_visited;  /* player stood here */
static unsigned char * ex_item;     /* an item was seen here */
static unsigned char * ex_skip;     /* locked door: skip */
static item_t ** ex_seen_items;     /* items already seen on this level */
static unsigned int ex_seen_n, ex_seen_cap;

/* Per-walk state. */
static unsigned int ex_msgs;
static unsigned int ex_py, ex_px;
static int ex_moved_check;
static int ex_door_step;
static int ex_avoid_mon = 1;   /* path around creatures in view */
static int ex_monsters;             /* stair walk: monsters in view at start */

#define IDX(y, x) ((y) * ex_sx + (x))

static void ex_reset_level(level_t * level)
{
  free(ex_visited); free(ex_item); free(ex_skip);
  ex_sy = level->size_y;
  ex_sx = level->size_x;
  ex_visited = calloc(ex_sy * ex_sx, 1);
  ex_item = calloc(ex_sy * ex_sx, 1);
  ex_skip = calloc(ex_sy * ex_sx, 1);
  ex_seen_n = 0;
  ex_level = level->level_index;
}

static void ex_sync_level(void)
{
  level_t * level = get_current_level();

  if (level != NULL && (ex_level != (int)level->level_index || ex_visited == NULL))
    ex_reset_level(level);
}

static int ex_item_seen(item_t * item)
{
  unsigned int i;

  for (i = 0; i < ex_seen_n; i++)
    if (ex_seen_items[i] == item)
      return 1;

  return 0;
}

static void ex_add_item(item_t * item)
{
  if (ex_seen_n == ex_seen_cap)
  {
    ex_seen_cap = ex_seen_cap ? ex_seen_cap * 2 : 64;
    ex_seen_items = realloc(ex_seen_items, ex_seen_cap * sizeof(item_t *));
  }

  ex_seen_items[ex_seen_n++] = item;
}

/*
  Records visible items. Returns 1 if an item came into view that was
  not seen before on this level.
*/
static int ex_scan_items(level_t * level)
{
  item_t * item;
  int fresh = 0;

  for (item = level->first_item; item != NULL; item = item->next_item)
  {
    if (!on_map(level, item->y, item->x) ||
	can_see(game->player, item->y, item->x) == false)
      continue;

    if (ex_item_seen(item))
      continue;

    ex_add_item(item);

    if (ex_visited[IDX(item->y, item->x)] == 0)
      ex_item[IDX(item->y, item->x)] = 1;

    fresh = 1;
  }

  return fresh;
}

/* A visible hostile creature (the first one found), or NULL. */
static creature_t * ex_monster_in_view(level_t * level, int * count)
{
  unsigned int i;
  creature_t * c;
  creature_t * first = NULL;
  int n = 0;

  for (i = 0; i < level->creatures; i++)
  {
    c = level->creature[i];

    if (c == NULL || is_player(c) || attr_current(c, attr_player_ally))
      continue;

    if (!on_map(level, c->y, c->x) ||
	can_see_creature(game->player, c) == false)
      continue;

    if (first == NULL)
      first = c;

    n++;
  }

  if (count)
    *count = n;

  return first;
}

static int ex_is_trap_gent(gent_t g)
{
  return (g >= gent_trap_generic && g <= gent_web) || g == gent_internal_trap;
}

/* Is (y, x) a known, safe cell to step on? */
static int ex_passable(level_t * level, unsigned int y, unsigned int x)
{
  gent_t g = level->memory[y][x];

  if (g == gent_blank)
    return 0;

  if (ex_skip[IDX(y, x)])
    return 0;

  if (ex_is_trap_gent(g) || g == gent_water || g == gent_lava ||
      g == gent_forcefield)
    return 0;

  /* Walk around creatures in view instead of attacking them. */
  if (ex_avoid_mon)
  {
    creature_t * c = find_creature(level, y, x);

    if (c != NULL && !is_player(c) &&
	can_see_creature(game->player, c))
      return 0;
  }

  if (g == gent_door_closed)
    return 1;

  return tile_info[level->map[y][x]]->walkable ? 1 : 0;
}

static int ex_is_stairs(level_t * level, unsigned int y, unsigned int x)
{
  tile_t t = level->map[y][x];

  return level->memory[y][x] == gent_stairs &&
    t >= tile_stair0 && t <= tile_stair3;
}

/* Depth of each level: link distance from the starting level. */
static int ex_depth(int index)
{
  int depth[LEVELS];
  int queue[LEVELS];
  int head = 0, tail = 0;
  int i, j;

  for (i = 0; i < LEVELS; i++)
    depth[i] = -1;

  depth[LEVEL_START] = 0;
  queue[tail++] = LEVEL_START;

  while (head < tail)
  {
    i = queue[head++];

    if (game->level_list[i] == NULL)
      continue;

    for (j = 0; j < STAIRS; j++)
    {
      int n = game->level_list[i]->link[j];

      if (n < 0 || n >= LEVELS || depth[n] != -1)
	continue;

      depth[n] = depth[i] + 1;
      queue[tail++] = n;
    }
  }

  return (index >= 0 && index < LEVELS) ? depth[index] : -1;
}

/* Does the staircase at (y, x) go the way MODE asks? */
static int ex_stairs_match(level_t * level, unsigned int y, unsigned int x, int mode)
{
  int target;

  if (!ex_is_stairs(level, y, x))
    return 0;

  if (mode == EXPLORE_STAIRS_ANY)
    return 1;

  target = level->link[level->map[y][x] - tile_stair0];

  if (mode == EXPLORE_STAIRS_UP)
    return ex_depth(target) < ex_depth(level->level_index);

  return ex_depth(target) >= ex_depth(level->level_index);
}

static int ex_is_frontier(level_t * level, unsigned int y, unsigned int x)
{
  int dy, dx;

  if (ex_item[IDX(y, x)])
    return 1;

  if (ex_visited[IDX(y, x)])
    return 0;

  for (dy = -1; dy <= 1; dy++)
    for (dx = -1; dx <= 1; dx++)
    {
      int ny = (int)y + dy, nx = (int)x + dx;

      if (ny < 0 || nx < 0 || ny >= (int)ex_sy || nx >= (int)ex_sx)
	continue;

      if (level->memory[ny][nx] == gent_blank)
	return 1;
    }

  return 0;
}

static const int ex_dy[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
static const int ex_dx[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };

/*
  BFS from the player over known passable cells. Returns the first
  step direction index (0-7) toward the nearest target, -1 if the
  player stands on a target, -2 if nothing is reachable. *blocked is
  set when a target exists behind a trap or locked door.
*/
static int ex_bfs_once(level_t * level, int mode, int * blocked);

/* Paths around creatures in view; if they block every way, through them. */
static int ex_bfs(level_t * level, int mode, int * blocked)
{
  int d;

  ex_avoid_mon = 1;
  d = ex_bfs_once(level, mode, blocked);

  if (d == -2)
  {
    ex_avoid_mon = 0;
    d = ex_bfs_once(level, mode, blocked);
    ex_avoid_mon = 1;
  }

  return d;
}

static int ex_bfs_once(level_t * level, int mode, int * blocked)
{
  unsigned int n = ex_sy * ex_sx;
  int * from = malloc(n * sizeof(int));
  int * queue = malloc(n * sizeof(int));
  int head = 0, tail = 0;
  int start = IDX(game->player->y, game->player->x);
  int found = -1;
  int i, d;

  *blocked = 0;

  for (i = 0; i < (int)n; i++)
    from[i] = -2;

  from[start] = -1;
  queue[tail++] = start;

  while (head < tail)
  {
    int cur = queue[head++];
    int cy = cur / ex_sx, cx = cur % ex_sx;
    int is_target;

    if (mode == EXPLORE_ON)
      is_target = ex_is_frontier(level, cy, cx) && !(cur == start);
    else
      is_target = ex_stairs_match(level, cy, cx, mode);

    if (is_target)
    {
      found = cur;
      break;
    }

    /* Don't walk on through stairs or closed doors, they are ends. */
    if (cur != start && level->memory[cy][cx] == gent_door_closed)
      continue;

    for (d = 0; d < 8; d++)
    {
      int ny = cy + ex_dy[d], nx = cx + ex_dx[d];
      int ni;

      if (ny < 0 || nx < 0 || ny >= (int)ex_sy || nx >= (int)ex_sx)
	continue;

      ni = IDX(ny, nx);

      if (from[ni] != -2)
	continue;

      if (!ex_passable(level, ny, nx))
      {
	gent_t g = level->memory[ny][nx];

	if (ex_is_trap_gent(g) || ex_skip[ni])
	  *blocked = 1;

	continue;
      }

      from[ni] = cur;
      queue[tail++] = ni;
    }
  }

  if (found == start && mode != EXPLORE_ON)
    d = -1;
  else if (found < 0)
    d = -2;
  else
  {
    int cur = found;

    while (from[cur] != start)
      cur = from[cur];

    for (d = 0; d < 8; d++)
      if (IDX(game->player->y + ex_dy[d], game->player->x + ex_dx[d]) == cur)
	break;
  }

  free(from);
  free(queue);

  return d;
}

static action_t ex_dir_action(int d)
{
  static const action_t act[8] = { action_n, action_s, action_w, action_e,
				   action_nw, action_ne, action_sw, action_se };
  return act[d];
}

static action_t ex_stop(const char * msg)
{
  explore_mode = EXPLORE_OFF;

  if (msg)
    queue_msg(msg);

  msgflush_nowait();

  return action_undefined;
}

void explore_cancel(void)
{
  explore_mode = EXPLORE_OFF;
}

/* Arms a walk. Returns false if nothing happens (a message says why). */
blean_t explore_start(const int mode)
{
  level_t * level = get_current_level();

  if (game->player == NULL || level == NULL)
    return false;

  ex_sync_level();

  explore_mode = mode;
  ex_msgs = msg_counter;
  ex_moved_check = 0;
  ex_door_step = 0;
  ex_py = game->player->y;
  ex_px = game->player->x;

  /* Items already in view at the key press don't stop the walk. */
  ex_scan_items(level);
  ex_monster_in_view(level, &ex_monsters);

  return true;
}

/*
  One step of an armed walk: returns a movement action for
  player_control() to carry out, or action_undefined when the walk
  stops (the reason is in the message bar).
*/
action_t explore_step(creature_t * player)
{
  level_t * level = get_current_level();
  creature_t * mon;
  int blocked;
  int d;
  int n;
  char line[100];

  if (explore_mode == EXPLORE_OFF || level == NULL || player == NULL)
    return action_undefined;

  ex_sync_level();

  ex_visited[IDX(player->y, player->x)] = 1;
  ex_item[IDX(player->y, player->x)] = 0;

  /* Paint the last step, then look for a key press. */
  draw_level();
  display_stats(player);
  msgflush_nowait();
#ifdef __EMSCRIPTEN__
  web_pause(40);

  if (web_poll_key() != -1)
    return ex_stop(NULL);
#endif

  if (ex_moved_check && player->y == ex_py && player->x == ex_px)
    return ex_stop(NULL);

  if (ex_door_step)
  {
    ex_door_step = 0;
    ex_msgs = msg_counter;
  }

  if (msg_counter != ex_msgs)
    return ex_stop(NULL);

  mon = ex_monster_in_view(level, &n);

  if (mon != NULL &&
      (explore_mode == EXPLORE_ON || n > ex_monsters))
  {
    sprintf(line, "In view: %s.", mon->name_one);
    return ex_stop(line);
  }

  if (ex_scan_items(level) && explore_mode == EXPLORE_ON)
    return ex_stop("You see something new.");

  d = ex_bfs(level, explore_mode, &blocked);

  if (d == -1)
    return ex_stop(NULL); /* on the stairs: press again to take them */

  if (d == -2)
  {
    if (explore_mode == EXPLORE_ON)
    {
      if (blocked)
	return ex_stop("Known traps or locked doors block the way on.");

      return ex_stop("Nothing left to explore (try searching walls).");
    }

    if (explore_mode == EXPLORE_STAIRS_UP)
      return ex_stop("You know no way up from here.");

    if (explore_mode == EXPLORE_STAIRS_DOWN)
      return ex_stop("You know no way down from here.");

    return ex_stop("You know no stairs here.");
  }

  {
    unsigned int ny = player->y + ex_dy[d];
    unsigned int nx = player->x + ex_dx[d];

    if (level->map[ny][nx] == tile_door_locked &&
	level->memory[ny][nx] == gent_door_closed)
    {
      ex_skip[IDX(ny, nx)] = 1;
      return ex_stop("The door is locked.");
    }

    if (level->memory[ny][nx] == gent_door_closed)
    {
      /* Opening takes the turn and doesn't move us; "You open the
	 door." must not stop the walk. */
      ex_moved_check = 0;
      ex_door_step = 1;
    }
    else
    {
      ex_moved_check = 1;
      ex_door_step = 0;
    }

    ex_msgs = msg_counter;
  }

  ex_py = player->y;
  ex_px = player->x;

  return ex_dir_action(d);
}


/* Is the player on stairs going the MODE way? */
blean_t explore_on_stairs(const int mode)
{
  level_t * level = get_current_level();

  if (level == NULL || game->player == NULL)
    return false;

  return ex_stairs_match(level, game->player->y, game->player->x, mode);
}
