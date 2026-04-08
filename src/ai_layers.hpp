#pragma once

#include <vector>

#include "ai_heuristics.hpp"

// Layer For Changing Weapons
inline static Bezier weapon_change_bezier = Bezier({50.0f, 0.0f}, {60.0f, 1.0f}, {65.0f, 0.75f}, {70.0f, 1.0f});
inline static std::vector<Line*> weapon_change_heuristic_lines = {&weapon_change_bezier};
inline static Heuristic_Layer weapon_change_layer = {weapon_change_heuristic_lines};

// Layer For Shotgun Range
inline static Bezier shotgun_dist_bezier = Bezier({1200.0f, 1.0f}, {1500.0f, 0.8f}, {1800.0f, 0.5f}, {2000.0f, 0.0f});
// Lines are auto-sorted based on start x
inline static std::vector<Line*> shotgun_dist_heuristic_lines = {&shotgun_dist_bezier};
inline static Heuristic_Layer shotgun_dist_layer = {shotgun_dist_heuristic_lines};

//Layer for Zombiemen
inline static StraightLine danger_score_dist_z = StraightLine({0.0f, 20.0f}, {1024.0f, 0.0f});
inline static std::vector<Line *> danger_score_zombie_heuristic_lines = {&danger_score_dist_z};
inline static Heuristic_Layer danger_score_zombie_layer = {danger_score_zombie_heuristic_lines};

// Layer for Imps
inline static StraightLine danger_score_dist_i = StraightLine({0.0f, 25.0f}, {1024.0f, 20.0f});
inline static std::vector<Line *> danger_score_imp_heuristic_lines = {&danger_score_dist_i};
inline static Heuristic_Layer danger_score_imp_layer = {danger_score_imp_heuristic_lines};

// Layer for Shotgunner
inline static StraightLine danger_score_dist_s = StraightLine({0.0f, 35.0f}, {1024.0f, 10.0f});
inline static std::vector<Line *> danger_score_shotguy_heuristic_lines = {&danger_score_dist_s};
inline static Heuristic_Layer danger_score_shotguy_layer = {danger_score_shotguy_heuristic_lines};


inline static Bezier shotgun_ammo_bezier = Bezier({0.0f, 0.0f}, {0.0f, 1.0f}, {3.0f, 0.80f}, {5.0f, 1.0f});
inline static std::vector<Line*> shotgun_ammo_heuristic_lines = {&shotgun_ammo_bezier};
inline static Heuristic_Layer shotgun_ammo_layer = {shotgun_ammo_heuristic_lines};
inline static Bezier pistol_ammo_bezier = Bezier({0.0f, 0.0f}, {3.0f, 0.2f}, {10.0, 1.0f}, {20.0f, 1.0f});
inline static std::vector<Line*> pistol_ammo_heuristic_lines = {&pistol_ammo_bezier};
inline static Heuristic_Layer pistol_ammo_layer = {pistol_ammo_heuristic_lines};


struct Shooting_Layers
{
    const Heuristic_Layer WEAPON_CHANGE_LAYER = weapon_change_layer;
    const Heuristic_Layer SHOTGUN_DISTANCE_LAYER = shotgun_dist_layer;
    const Heuristic_Layer SHOTGUN_AMMO_LAYER = shotgun_ammo_layer;
    const Heuristic_Layer PISTOL_AMMO_LAYER = pistol_ammo_layer;
    const int PISTOL_QUICK_FIRE_MAX_DIST = 750;
};

static const Shooting_Layers shooting_layers;

inline static Bezier targetDistancePosLine = Bezier{{0.0f, 0.25f}, {256.0f, 1.0f}, {512.0f, 0.8f}, {1024.0f, 0.0f}};
inline static Heuristic_Layer TARGET_DISTANCE_POS_LAYER = {std::vector<Line*>{&targetDistancePosLine}};

inline static StraightLine zombieDistancePosLine1 = StraightLine{{0.0f, 2.0f}, {192.0f, 1.0f}};
inline static StraightLine zombieDistancePosLine2 = StraightLine{{192.0f, 1.0f}, {384.0f, 0.25f}};
inline static StraightLine zombieDistancePosLine3 = StraightLine{{384.0f, 0.25f}, {768.0f, 0.0f}};
inline static Heuristic_Layer ZOMBIE_DISTANCE_POS_LAYER = {std::vector<Line*>{&zombieDistancePosLine1, &zombieDistancePosLine2, &zombieDistancePosLine3}};

inline static StraightLine shotgunnerDistancePosLine1 = StraightLine{{0.0f, 5.0f}, {256.0f, 3.0f}};
inline static StraightLine shotgunnerDistancePosLine2 = StraightLine{{256.0f, 3.0f}, {768.0f, 0.5f}};
inline static Heuristic_Layer SHOTGUNNER_DISTANCE_POS_LAYER = {std::vector<Line*>{&shotgunnerDistancePosLine1, &shotgunnerDistancePosLine2}};

inline static StraightLine impDistancePosLine1 = StraightLine{{0.0f, 3.0f}, {192.0f, 1.0f}};
inline static StraightLine impDistancePosLine2 = StraightLine{{192.0f, 1.0f}, {512.0f, 0.5f}};
inline static Heuristic_Layer IMP_DISTANCE_POS_LAYER = {std::vector<Line*>{&impDistancePosLine1, &impDistancePosLine2}};

inline static Bezier pathDistancePosLine = Bezier{{0.0f, 1.0f}, {384.0f, 0.8f}, {768.0f, 0.3f}, {1024.0f, 0.0f}};
inline static Heuristic_Layer PATH_DISTANCE_POS_LAYER = {std::vector<Line*>{&pathDistancePosLine}};

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