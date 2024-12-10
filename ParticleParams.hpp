#pragma once

#include "const.hpp"

template<typename P_TYPE, typename V_TYPE, typename V_FLOW_TYPE>
class Fluid;

template<typename P_TYPE, typename V_TYPE>
struct ParticleParams {
    char type;
    P_TYPE cur_p;
    std::array<V_TYPE, deltas.size()> v;

    template<typename V_FLOW_TYPE>
    void swap_with(Fluid<P_TYPE, V_TYPE, V_FLOW_TYPE>& f, int x, int y) {
        std::swap((*f.field)[x][y], type);
        std::swap((*f.p)[x][y], cur_p);
        std::swap((*f.velocity.v)[x][y], v);
    }
};
