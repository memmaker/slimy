#ifndef _EXPLORE_H_
#define _EXPLORE_H_

/* RVIP auto-explore and walk-to-stairs (see explore.c). */

#include "main.h"
#include "input.h"

#define EXPLORE_OFF         0
#define EXPLORE_ON          1
#define EXPLORE_STAIRS_UP   2
#define EXPLORE_STAIRS_DOWN 3
#define EXPLORE_STAIRS_ANY  4

extern int explore_mode;

blean_t explore_start(const int mode);
action_t explore_step(creature_t * player);
void explore_cancel(void);
/* Link distance of level INDEX from the start level (-1 = unreachable). */
int explore_depth(int index);
blean_t explore_on_stairs(const int mode);

#endif
