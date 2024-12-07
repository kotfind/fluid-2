#pragma once

#include "Fixed.hpp"
#include "const.hpp"

class Fluid;

struct ParticleParams {
    char type;
    Fixed cur_p;
    std::array<Fixed, deltas.size()> v;

    void swap_with(Fluid&, int x, int y);
};
