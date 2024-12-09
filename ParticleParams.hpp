#pragma once

#include "const.hpp"

template<typename T>
class Fluid;

template<typename T>
struct ParticleParams {
    char type;
    T cur_p;
    std::array<T, deltas.size()> v;

    void swap_with(Fluid<T>& f, int x, int y) {
        std::swap((*f.field)[x][y], type);
        std::swap((*f.p)[x][y], cur_p);
        std::swap((*f.velocity.v)[x][y], v);
    }
};
