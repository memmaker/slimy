#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "stuff.h"
#include "wingame.h"
#include "options.h"
#include "game.h"
#include "message.h"
#include "losegame.h"
#include "ui.h"
#ifdef __EMSCRIPTEN__
extern void web_end(void);
void web_autosave_delete(void);
#endif


void win_game(const unsigned int ending)
{
  queue_msg("You ascend to demigodhood.");
  msgflush_wait();
  
  game->died = time(NULL);
  game->game_over = true;
  game->won = true;

  run_report("win", NULL);   /* RVIP stage 9: before the game-over key wait */
#ifdef __EMSCRIPTEN__
  web_autosave_delete();     /* RVIP: the run is over, no autosave left */
#endif
  
  if (options.morgue)
  {
    morgue_dump("ascended to demigodhood");
  }

  game_over("You have won!\n", true);
  shutdown_everything();

#ifdef __EMSCRIPTEN__
  web_end();
#endif
  exit(0);
} /* win_game */
