#pragma once

#include "const.hpp"

template<size_t N, size_t M, typename T>
class Fluid;

template<typename T>
struct ParticleParams {
    char type;
    T cur_p;
    std::array<T, deltas.size()> v;

    template<size_t N, size_t M>
    void swap_with(Fluid<N, M, T>& f, int x, int y) {
        std::swap(f.field[x][y], type);
        std::swap(f.p[x][y], cur_p);
        std::swap(f.velocity.v[x][y], v);
    }
};
