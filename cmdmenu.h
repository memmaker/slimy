#ifndef _CMDMENU_H_
#define _CMDMENU_H_

/* RVIP: floating command menu on Enter (see cmdmenu.c). */

#include "input.h"

int command_menu(action_t * action);

/* Set by dwim_inventory() when an item action ended it; reopens the list. */
extern int inv_reopen;

#endif
