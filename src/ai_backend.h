#ifndef AI_BACKEND_H
#define AI_BACKEND_H

#include "d_player.h"

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C void AI_Tick(player_t* player);

#endif
