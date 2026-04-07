#pragma once

#include <vector>

#include "ai_heuristics.hpp"

// Layer For Changing Weapons
static Bezier weapon_change_bezier = Bezier({20.0f, 0.0f}, {20.0f, 1.0f}, {25.0f, 0.75f}, {30.0f, 1.0f});
static std::vector<Line*> weapon_change_heuristic_lines = {&weapon_change_bezier};
static Heuristic_Layer weapon_change_layer = {weapon_change_heuristic_lines};

// Layer For Shotgun Range
static Bezier shotgun_dist_bezier = Bezier({800.0f, 1.0f}, {1200.0f, 0.85f}, {1500.0f, 0.5f}, {1800.0f, 0.0f});
// Lines are auto-sorted based on start x
static std::vector<Line*> shotgun_dist_heuristic_lines = {&shotgun_dist_bezier};
static Heuristic_Layer shotgun_dist_layer = {shotgun_dist_heuristic_lines};

//Layer for Zombiemen
static StraightLine danger_score_dist_z = StraightLine({0.0f, 20.0f}, {1024.0f, 0.0f});
static std::vector<Line *> danger_score_zombie_heuristic_lines = {&danger_score_dist_z};
static Heuristic_Layer danger_score_zombie_layer = {danger_score_zombie_heuristic_lines};

// Layer for Imps
static StraightLine danger_score_dist_i = StraightLine({0.0f, 25.0f}, {1024.0f, 20.0f});
static std::vector<Line *> danger_score_imp_heuristic_lines = {&danger_score_dist_i};
static Heuristic_Layer danger_score_imp_layer = {danger_score_imp_heuristic_lines};

// Layer for Shotgunner
static StraightLine danger_score_dist_s = StraightLine({0.0f, 35.0f}, {1024.0f, 10.0f});
static std::vector<Line *> danger_score_shotguy_heuristic_lines = {&danger_score_dist_s};
static Heuristic_Layer danger_score_shotguy_layer = {danger_score_shotguy_heuristic_lines};


struct Shooting_Layers
{
    const Heuristic_Layer WEAPON_CHANGE_LAYER = weapon_change_layer;
    const Heuristic_Layer SHOTGUN_DISTANCE_LAYER = shotgun_dist_layer;
};

static const Shooting_Layers shooting_layers;

static Bezier targetDistancePosLine = Bezier{{0.0f, 0.25f}, {256.0f, 1.0f}, {512.0f, 0.8f}, {1024.0f, 0.0f}};
static Heuristic_Layer TARGET_DISTANCE_POS_LAYER = {std::vector<Line*>{&targetDistancePosLine}};

static StraightLine zombieDistancePosLine1 = StraightLine{{0.0f, 2.0f}, {192.0f, 1.0f}};
static StraightLine zombieDistancePosLine2 = StraightLine{{192.0f, 1.0f}, {384.0f, 0.25f}};
static StraightLine zombieDistancePosLine3 = StraightLine{{384.0f, 0.25f}, {768.0f, 0.0f}};
static Heuristic_Layer ZOMBIE_DISTANCE_POS_LAYER = {std::vector<Line*>{&zombieDistancePosLine1, &zombieDistancePosLine2, &zombieDistancePosLine3}};

static StraightLine shotgunnerDistancePosLine1 = StraightLine{{0.0f, 5.0f}, {256.0f, 3.0f}};
static StraightLine shotgunnerDistancePosLine2 = StraightLine{{256.0f, 3.0f}, {768.0f, 0.5f}};
static Heuristic_Layer SHOTGUNNER_DISTANCE_POS_LAYER = {std::vector<Line*>{&shotgunnerDistancePosLine1, &shotgunnerDistancePosLine2}};

static StraightLine impDistancePosLine1 = StraightLine{{0.0f, 3.0f}, {192.0f, 1.0f}};
static StraightLine impDistancePosLine2 = StraightLine{{192.0f, 1.0f}, {512.0f, 0.5f}};
static Heuristic_Layer IMP_DISTANCE_POS_LAYER = {std::vector<Line*>{&impDistancePosLine1, &impDistancePosLine2}};

static StraightLine fireDistancePosLine1 = StraightLine{{0.0f, 5.0f}, {128.0f, 1.0f}};
static StraightLine fireDistancePosLine2 = StraightLine{{128.0f, 1.0f}, {256.0f, 0.0f}};
static Heuristic_Layer FIRE_DISTANCE_POS_LAYER = {std::vector<Line*>{&fireDistancePosLine1, &fireDistancePosLine2}};

static Bezier pathDistancePosLine = Bezier{{0.0f, 1.0f}, {384.0f, 0.8f}, {768.0f, 0.3f}, {1024.0f, 0.0f}};
static Heuristic_Layer PATH_DISTANCE_POS_LAYER = {std::vector<Line*>{&pathDistancePosLine}};

// FOR DEBUGGING HEURISTICS
// void Debug_Heuristic_Layers() {
//     for (int i = 0; i < 36; i++)
//     {
//         printf("Frame %d: %f\n", i, shooting_layers.WEAPON_CHANGE_LAYER.Get_Current_Val(i));
//     }
//
//     for (int i = 0; i < 1001; i += 50)
//     {
//         printf("Frame %d: %f\n", i, shooting_layers.SHOTGUN_DISTANCE_LAYER.Get_Current_Val(i));
//     }
// }