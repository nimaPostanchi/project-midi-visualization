#pragma once

#include <iostream>


struct Position2D final
{
    unsigned x, y;

    Position2D(unsigned x, unsigned y)
        : x(x), y(y) { }

    Position2D move(int dx, int dy) const
    {
        return Position2D(unsigned(x + dx), unsigned(y + dy));
    }
};

inline std::ostream& operator <<(std::ostream& out, const Position2D& p)
{
    return out << "(" << p.x << "," << p.y << ")";
}
