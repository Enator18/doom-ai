#ifndef AI_BACKEND_H
#define AI_BACKEND_H

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

typedef struct player_s player_t;

EXTERN_C void AI_Init();

EXTERN_C void AI_Tick(player_t* player);

#endif
