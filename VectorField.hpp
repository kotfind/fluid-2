#pragma once

#include "Fixed.hpp"
#include "const.hpp"

#include <array>

struct VectorField {
    std::array<Fixed, deltas.size()> v[N][M];

    Fixed& add(int x, int y, int dx, int dy, Fixed dv);
    Fixed& get(int x, int y, int dx, int dy);
};
