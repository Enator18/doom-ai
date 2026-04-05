#include <algorithm>
#include <vector>
#include <ranges>

#include "ai_heuristics.hpp"

StraightLine::StraightLine(const Point &p0, const Point &p1)
{
    this->p0 = p0;
    this->p1 = p1;
    this->start_x = p0.x;
    this->end_x = p1.x;
    this->slope = (p0.y - p1.y) / (p0.x - p1.x);
}

float StraightLine::Get_Val_At(float x)
{
    if (!X_Within_Bounds(x)) return -1;

    return std::max(p0.y + slope * (x - p0.x), 0.0f);
}

Bezier::Bezier(const Point& p0, const Point& p1, const Point& p2, const Point& p3)
{
    this->p0 = p0;
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    this->start_x = p0.x;
    this->end_x = p3.x;
}

const int BINARY_SEARCH_ITER = 30;

float Bezier::Get_Val_At(float x)
{
    if (!X_Within_Bounds(x)) return -1;

    float target_x = x;

    float low = 0.0f, high = 1.0f, t = 0.5f;
    for (int i = 0; i < BINARY_SEARCH_ITER; i++)
    {
        float u = 1.0f - t;
        float x = p0.x * u * u * u
                + p1.x * 3 * u * u * t
                + p2.x * 3 * u * t * t
                + p3.x * t * t * t;

        if (x < target_x)
        {
            low = t;
        }
        else
        {
            high = t;
        }

        t = (low + high) * 0.5f;
    }

    float u = 1.0f - t;
    return p0.y * u * u * u
         + p1.y * 3 * u * u * t
         + p2.y * 3 * u * t * t
         + p3.y * t * t * t;
}

Heuristic_Layer::Heuristic_Layer(std::vector<Line*> lines)
{
    this->lines = lines;
    std::ranges::sort(this->lines, [](const Line* a, const Line* b) {
        return a->Get_Start_Bound() < b->Get_Start_Bound();
    });
}

float Heuristic_Layer::Get_Current_Val(float x) const
{
    float current_heuristic = lines[0]->Get_Val_At(lines[0]->Get_Start_Bound());
    for (int i = 0; i < this->lines.size(); i++)
    {
        if (lines[i]->Before_Start_Bound(x))
        {
            break;
        }

        if (!lines[i]->After_End_Bound(x))
        {
            current_heuristic = lines[i]->Get_Val_At(x);
            break;
        }

        current_heuristic = lines[i]->Get_Val_At(lines[i]->Get_End_Bound());
    }

    return current_heuristic;
}

