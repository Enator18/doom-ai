# pragma once

#include <vector>

struct Point
{
    float x, y;

    Point operator *(float num) const
    {
        return {x * num, y * num};
    }

    void operator =(const Point& point)
    {
        this->x = point.x;
        this->y = point.y;
    }
};

class Line
{
public:
    virtual float Get_Val_At(float x) = 0;

    int Get_Start_Bound() const
    {
        return this->start_x;
    }

    int Get_End_Bound() const
    {
        return this->end_x;
    }

    bool Before_Start_Bound(float x)
    {
        return x < this->start_x;
    }

    bool After_End_Bound(float x)
    {
        return x > this->end_x;
    }

protected:
    bool X_Within_Bounds(float x)
    {
        return  this->start_x <= x && this->end_x >= x;
    }

    float start_x = 0;
    float end_x = 0;
};

class StraightLine: public Line
{
public:
    StraightLine(const Point& p0, const Point& p1);
    float Get_Val_At(float x);
private:
    Point p0;
    Point p1;
    float slope;
};

class Bezier: public Line
{
public:
    Bezier(const Point& p0, const Point& p1, const Point& p2, const Point& p3);
    float Get_Val_At(float x);

private:
    Point p0;
    Point p1;
    Point p2;
    Point p3;
};

class Heuristic_Layer
{
public:
    Heuristic_Layer(std::vector<Line*> lines);
    float Get_Current_Val(float x) const;

protected:
    std::vector<Line*> lines;
};