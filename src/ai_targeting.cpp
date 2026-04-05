#include <stdint.h>
#include <cmath>
#include <cstdio>
#include <numeric>

#include "ai_targeting.hpp"
#include "ai_layers.hpp"
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

void AI_Targeting::Decide_To_Switch_Weapon(player_t *player)
{
    float dist_layer = shooting_layers.SHOTGUN_DISTANCE_LAYER.Get_Current_Val(Average_Enemy_Dist(player));
    float switch_layer = shooting_layers.WEAPON_CHANGE_LAYER.Get_Current_Val(ticks_since_swap);

    if (player->readyweapon != wp_shotgun && switch_layer * dist_layer >= 0.4f && player->ammo[am_shell] > 0)
    {
        Switch_Weapon(wp_shotgun);
    }

    if (player->readyweapon != wp_pistol && switch_layer * (1 - dist_layer) >= 0.6f && player->ammo[am_clip] > 0) {
        Switch_Weapon(wp_pistol);
    }

    if (player->readyweapon == wp_shotgun && player->ammo[am_shell] == 0) {
        Switch_Weapon(wp_pistol);
    }

    if (player->readyweapon == wp_pistol && player->ammo[am_clip] == 0) {
        Switch_Weapon(wp_shotgun);
    }
}

float AI_Targeting::Average_Enemy_Dist(player_t *player) {
    this->enemies = Get_Enemies(player);

    if (enemies.size() == 0)
    {
        return float(std::numeric_limits<float>::max());
    }

    std::vector<float> closest_n_distances(MAX_NEARBY_ENEMIES);

    for (int i = 0; i < enemies.size(); i++) {
        float distance = Entity_Float_Distance(player->mo, enemies[i]);
        if (closest_n_distances.size() < MAX_NEARBY_ENEMIES) {
            closest_n_distances.push_back(distance);
        } else {
            for (int j = 0; j < closest_n_distances.size(); j++) {
                if (distance > closest_n_distances[j]) {
                    closest_n_distances[j] = distance;
                }
            }
        }
    }

    closest_n_distances[0] = closest_n_distances[0] * 0.6f;
    closest_n_distances[1] = closest_n_distances[1] * 0.8f;

    return std::accumulate(closest_n_distances.begin(), closest_n_distances.end(), 0.0) / closest_n_distances.size();
}

mobj_t *AI_Targeting::Get_Closest_Enemy(float& dist_from_enemy)
{
    this->enemies = Get_Enemies(player);

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
        Decide_To_Switch_Weapon(player);
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
        //
    }
}

bool AI_Targeting::In_Shotgun_Distance(mobj_t *target)
{
    return Entity_Float_Distance(player->mo, target) < SHOTGUN_MAX_DIST;
}

bool AI_Targeting::Should_Use_Weapon(int ammo, bool requirement)
{
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

    ticks_since_swap = 0;
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

    // Remove Dead Enemies From Selection
    std::erase_if(enemies, [](const mobj_t *ptr) { return ptr->health <= 0; });

    return enemies;
}

float AI_Targeting::Entity_Float_Distance(mobj_t *start, mobj_t *end)
{
    return std::sqrt(
        std::pow(FixedToFloat(start->x) - FixedToFloat(end->x), 2)
        + std::pow(FixedToFloat(start->y) - FixedToFloat(end->y), 2));
}
