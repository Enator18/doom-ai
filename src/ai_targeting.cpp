#include <stdint.h>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <iostream>

#include "ai_targeting.hpp"
#include "ai_layers.hpp"
#include "ai_utils.hpp"


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
}

AI_Targeting::~AI_Targeting()
{
}

void AI_Targeting::Decide_To_Switch_Weapon(player_t *player)
{

    float pistol_ammo = player->ammo[am_clip];
    float shotgun_ammo = player->ammo[am_shell];

    float dist_layer = shooting_layers.SHOTGUN_DISTANCE_LAYER.Get_Current_Val(Average_Enemy_Dist(player));
    float switch_layer = shooting_layers.WEAPON_CHANGE_LAYER.Get_Current_Val(ticks_since_swap);

    float pistol_ammo_layer = shooting_layers.PISTOL_AMMO_LAYER.Get_Current_Val(pistol_ammo);
    float shotgun_ammo_layer = shooting_layers.SHOTGUN_AMMO_LAYER.Get_Current_Val(shotgun_ammo);

    bool owns_shotgun = player->weaponowned[2];

    float pistol_heuristic = (1 - dist_layer) * pistol_ammo_layer;
    float shotgun_heuristic = dist_layer * shotgun_ammo_layer;

    if (ticks_since_swap >= 35) {
        if (player->readyweapon != wp_shotgun && shotgun_heuristic * switch_layer > pistol_heuristic + 0.5f)
        {

            if (owns_shotgun) Switch_Weapon(wp_shotgun);
        }

        if (player->readyweapon != wp_pistol && pistol_heuristic * switch_layer > shotgun_heuristic + 0.05f) {
            Switch_Weapon(wp_pistol);
        }
    }

    if (player->readyweapon == wp_shotgun && player->ammo[am_shell] == 0) {
        Switch_Weapon(wp_pistol);
    }

    if (player->readyweapon == wp_pistol && player->ammo[am_clip] == 0) {
        Switch_Weapon(wp_shotgun);
    }
}

float AI_Targeting::Average_Enemy_Dist(player_t *player) {
    if (enemies.size() == 0)
    {
        return float(std::numeric_limits<float>::max());
    }

    std::vector<float> closest_n_distances;

    for (int i = 0; i < enemies.size(); i++) {
        float distance = Entity_Float_Distance(player->mo, enemies[i]);
        if (closest_n_distances.size() < MAX_NEARBY_ENEMIES) {
            closest_n_distances.push_back(distance);
        } else {
            for (int j = 0; j < closest_n_distances.size(); j++) {
                if (distance < closest_n_distances[j]) {
                    closest_n_distances[j] = distance;
                }
            }
        }
    }

    return std::accumulate(closest_n_distances.begin(), closest_n_distances.end(), 0.0) / closest_n_distances.size();
}

mobj_t *AI_Targeting::Get_Closest_Enemy(float& dist_from_enemy)
{

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

        if (player->readyweapon == wp_pistol)
        {
            Fire_Pistol(dist_from_enemy);
        }
        else
        {
            Shoot_Weapon();
        }
    }
}

mobj_t *last_target;

mobj_t *AI_Targeting::Get_Target_Enemy(float &dist_from_enemy)
{
    if (enemies.size() == 0)
    {
        return nullptr;
    }

    mobj_t *target_enemy = nullptr;
    float highest_score = 0;
    float target_distance = 0;

    for (int i = 0; i < enemies.size(); i++)
    {
        float distance = Entity_Float_Distance(player->mo, enemies[i]);
        float score = 0;

        if (enemies[i]->type == MT_POSSESSED)
        {
            score = danger_score_zombie_layer.Get_Current_Val(distance);
        }
        else if (enemies[i]->type == MT_TROOP)
        {
            score = danger_score_imp_layer.Get_Current_Val(distance);
        }
        else if (enemies[i]->type == MT_SHOTGUY)
        {
            score = danger_score_shotguy_layer.Get_Current_Val(distance);
        }

        if (last_target == enemies[i])
        {
            score += 8;
        }

        //std::cout << "score is " << score << std::endl;

        if (score > highest_score && P_CheckSight_12(player->mo, enemies[i]))
        {
            highest_score = score;
            target_enemy = enemies[i];
            target_distance = distance;
        }
    }

    dist_from_enemy = target_distance;
    last_target = target_enemy;

    //std::cout << target_enemy << std::endl;
    return target_enemy;
}

void AI_Targeting::Shoot_Target_Enemy(player_t *player)
{
    float dist_from_enemy;
    mobj_t *enemy = Get_Target_Enemy(dist_from_enemy);

    if (enemy != nullptr && dist_from_enemy < MAX_SHOOTING_RANGE)
    {
        PlayerLookAt(player, FixedToFloat(enemy->x), FixedToFloat(enemy->y));
        Decide_To_Switch_Weapon(player);
        player->cmd.buttons |= BT_ATTACK;
    }
}

void AI_Targeting::Shoot_Enemy(player_t *player, mobj_t* enemy)
{
    float dist_from_enemy = Entity_Float_Distance(player->mo, enemy);

    if (enemy != nullptr && dist_from_enemy < MAX_SHOOTING_RANGE)
    {
        PlayerLookAt(player, FixedToFloat(enemy->x), FixedToFloat(enemy->y));
        Decide_To_Switch_Weapon(player);
        player->cmd.buttons |= BT_ATTACK;
    }
}

void AI_Targeting::Fire_Pistol(float dist_to_target_enemy)
{
    if (dist_to_target_enemy > shooting_layers.PISTOL_QUICK_FIRE_MAX_DIST)
    {
        if (pistol_should_fire)
        {
            Shoot_Weapon();
            pistol_should_fire = false;
        }
    }
    else
    {
        Shoot_Weapon();
    }
}

void AI_Targeting::Shoot_Weapon()
{
    player->cmd.buttons |= BT_ATTACK;
}

void AI_Targeting::Choose_Weapon(mobj_t *target)
{
    // if (Should_Use_Weapon(am_shell, In_Shotgun_Distance(target)))
    // {
    //     Switch_Weapon(wp_shotgun);
    // }
    // else if (Should_Use_Weapon(am_clip, true))
    // {
    //     Switch_Weapon(wp_pistol);
    // }
    // else
    // {
    //     //
    // }
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
    if (player->readyweapon == weapon || player->pendingweapon == weapon)
    {
        return;
    }

    player->cmd.buttons |= BT_CHANGE | (weapon << BT_WEAPONSHIFT);
    ticks_since_swap = 0;
}

player_t *player;

float AI_Targeting::Entity_Float_Distance(mobj_t *start, mobj_t *end)
{
    return std::sqrt(
        std::pow(FixedToFloat(start->x) - FixedToFloat(end->x), 2)
        + std::pow(FixedToFloat(start->y) - FixedToFloat(end->y), 2));
}
