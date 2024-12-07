#include "ParticleParams.hpp"

#include "Fluid.hpp"

void ParticleParams::swap_with(Fluid& f, int x, int y) {
    std::swap(f.field[x][y], type);
    std::swap(f.p[x][y], cur_p);
    std::swap(f.velocity.v[x][y], v);
}
