#ifndef AI_TARGETING_H
#define AI_TARGETING_H

#include <vector>
#include "ai_heuristics.hpp"

extern "C"
{
#include "d_player.h"
#include "p_mobj.h"
}

class AI_Targeting {

  public:
    AI_Targeting(player_t *player);
    ~AI_Targeting();
    mobj_t *Get_Closest_Enemy(float& enemy_dist_out);
    void Shoot_Closest_Enemy(player_t *player);
    std::vector<mobj_t *> Get_Enemies(player_t *player);

    int ticks_since_swap = 0;

  private:
    bool Is_Weapon_Valid(bool weapon);
    void Choose_Weapon(mobj_t *target);
    bool Should_Use_Weapon(int ammo, bool requirement);
    void Switch_Weapon(int weapon);
    static float Entity_Float_Distance(mobj_t *start, mobj_t *end);
    bool In_Shotgun_Distance(mobj_t *target);
    float Average_Enemy_Dist(player_t *player);
    void Decide_To_Switch_Weapon(player_t *player);

    player_t *player;
    std::vector<mobj_t *> enemies;

    const int SHOTGUN_MAX_DIST = 500;
    const int MAX_SHOOTING_RANGE = 1250;
    const int MAX_NEARBY_ENEMIES = 3;
};

#endif
