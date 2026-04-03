#include <stdint.h>
#include <cmath>

#include "ai_targeting.hpp"
#include "ai_utils.h"

#include <vector>

extern "C"
{
#include "d_event.h"
#include "d_player.h"
#include "doomdef.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "r_defs.h"
#include "r_state.h"
    #include "d_items.h"
}

AI_Targeting::AI_Targeting(player_t *player)
{
    this->player = player;
    this->enemies = Get_Enemies(player);
}

AI_Targeting::~AI_Targeting()
{
}

mobj_t *AI_Targeting::Get_Closest_Enemy(float& dist_from_enemy)
{
    this->enemies = Get_Enemies(player);

    // Remove Dead Enemies From Selection
    std::erase_if(enemies, [](const mobj_t *ptr) { return ptr->health <= 0; });

    if (enemies.size() == 0)
    {
        return nullptr;
    }

    mobj_t *closest_enemy = nullptr;
    float closest_dist = float(std::numeric_limits<float>::max());

    for (int i = 0; i < enemies.size(); i++)
    {
        float distance = Entity_Float_Distance(player->mo, enemies[i]);

        if (distance < closest_dist && P_CheckSight_12(enemies[i], player->mo))
        {
            closest_dist = distance;
            closest_enemy = enemies[i];
        }
    }

    dist_from_enemy = closest_dist;

    return closest_enemy;
}

void AI_Targeting::Shoot_Closest_Enemy(player_t *player)
{
    float dist_from_enemy;
    mobj_t *enemy = Get_Closest_Enemy(dist_from_enemy);


    if (enemy != nullptr && dist_from_enemy < MAX_SHOOTING_RANGE)
    {
        PlayerLookAt(player, FixedToFloat(enemy->x), FixedToFloat(enemy->y));
        Choose_Weapon(enemy);
        player->cmd.buttons |= BT_ATTACK;
    }
}

void AI_Targeting::Choose_Weapon(mobj_t *target)
{
    if (Should_Use_Weapon(am_shell, In_Shotgun_Distance(target)))
    {
        Switch_Weapon(wp_shotgun);
    }
    else if (Should_Use_Weapon(am_clip, true))
    {
        Switch_Weapon(wp_pistol);
    }
    else
    {
        printf("No Ammo\n");
    }
}

bool AI_Targeting::In_Shotgun_Distance(mobj_t *target)
{
    return Entity_Float_Distance(player->mo, target) < SHOTGUN_MAX_DIST;
}

bool AI_Targeting::Should_Use_Weapon(int ammo, bool requirement)
{
    int ammo_total = player->ammo[ammo];
    printf("Ammo: %d\n", ammo_total);
    return player->ammo[ammo] > 0 && requirement;
}

void AI_Targeting::Switch_Weapon(int weapon)
{
    if (player->readyweapon == weapon)
    {
        return;
    }

    player->readyweapon = (weapontype_t)weapon;
    player->pendingweapon = wp_nochange;

    statenum_t state = (statenum_t)weaponinfo[weapon].readystate;
    P_Set_Player_Sprite(player, ps_weapon, state);
}

player_t *player;
std::vector<mobj_t *> enemies;
mobj_t *last_seen_enemy;

std::vector<mobj_t *> AI_Targeting::Get_Enemies(player_t *player)
{
    std::vector<mobj_t *> enemies;
    for (int i = 0; i < numsectors; i++)
    {
        sector_t sector = sectors[i];
        mobj_t *thing_ptr = sector.thinglist;
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

float AI_Targeting::Entity_Float_Distance(mobj_t *start, mobj_t *end)
{
    return std::sqrt(
        std::pow(FixedToFloat(start->x) - FixedToFloat(end->x), 2)
        + std::pow(FixedToFloat(start->y) - FixedToFloat(end->y), 2));
}
