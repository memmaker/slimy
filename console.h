#ifndef TSL_CONSOLE
#error TSL_CONSOLE not set
#endif

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <curses.h>

#include "main.h"
#include "gent.h"

WINDOW * board_win;
WINDOW * status_win;
WINDOW * message_bar;

blean_t st_reversed;

#endif
