#pragma once

#include "VectorField.hpp"

class ParticleParams;

class Fluid {
    public:

        Fluid();

        void run();

    private:
        std::tuple<Fixed, bool, std::pair<int, int>> propagate_flow(int x, int y, Fixed lim);

        void propagate_stop(int x, int y, bool force = false);

        Fixed move_prob(int x, int y);

        bool propagate_move(int x, int y, bool is_first);

        char field[N][M + 1];

        Fixed rho[256];

        Fixed p[N][M];
        Fixed old_p[N][M];

        VectorField velocity;
        VectorField velocity_flow;
        int last_use[N][M];
        int UT;

        Fixed g;

        int dirs[N][M];

    friend ParticleParams;
};
