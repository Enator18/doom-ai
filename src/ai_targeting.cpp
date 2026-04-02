#include <stdint.h>

#include "ai_targeting.hpp"
#include "ai_backend.h"

#include <vector>


extern "C"
{
    #include "p_mobj.h"
    #include "r_defs.h"
    #include "p_maputl.h"
    #include "r_state.h"
    #include "d_player.h"
    #include "r_main.h"
    #include "d_event.h"

    #include "p_maputl.h"
}

    AI_Targeting::AI_Targeting(player_t* player)
    {
        this->player = player;
        this->enemies = Get_Enemies(player);
    }

    AI_Targeting::~AI_Targeting(){}

    mobj_t* AI_Targeting::Get_Closest_Enemy()
    {
        this->enemies = Get_Enemies(player);
        // Remove Dead Enemies From Selection
        std::erase_if(enemies, [](const mobj_t* ptr) {
            return ptr->health <= 0;
        });

        if (enemies.size() == 0)
        {
            printf("%zu", enemies.size());
            return nullptr;
        }

        mobj_t* closest_enemy = nullptr;
        float closest_dist = float(std::numeric_limits<float>::max());

        for (int i = 0; i < enemies.size(); i++)
        {
            float distance = std::sqrt(
                std::pow(player->mo->x - enemies[i]->x, 2) +
                std::pow(player->mo->y - enemies[i]->y, 2)
            );

            if (distance < closest_dist &&
                P_CheckSight_12(enemies[i], player->mo))
            {
                closest_dist = distance;
                closest_enemy = enemies[i];
            }
        }

        return closest_enemy;
    }

    player_t* player;
    std::vector<mobj_t*> enemies;
    mobj_t* last_seen_enemy;

    std::vector<mobj_t*> AI_Targeting::Get_Enemies(player_t* player)
    {
        std::vector<mobj_t*> enemies;
        for (int i = 0; i < numsectors; i++)
        {
            sector_t sector = sectors[i];
            mobj_t* thing_ptr = sector.thinglist;
            while (thing_ptr != nullptr)
            {
                if (thing_ptr->target == player->mo)
                {
                    enemies.push_back(thing_ptr);
                }
                thing_ptr = thing_ptr->snext;
            }
        }
        return enemies;
    }






