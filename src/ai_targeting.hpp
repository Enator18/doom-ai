#ifndef AI_TARGETING_H
#define AI_TARGETING_H

#include <vector>

extern "C"
{
    #include "d_player.h"
    #include "p_mobj.h"
}

class AI_Targeting
{

public:
    AI_Targeting(player_t* player);
    ~AI_Targeting();
    mobj_t* Get_Closest_Enemy();

private:
    std::vector<mobj_t*> Get_Enemies(player_t* player);

    player_t* player;
    std::vector<mobj_t*> enemies;
};


#endif
