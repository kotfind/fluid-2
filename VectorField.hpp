#pragma once

#include "const.hpp"

#include <array>
#include <algorithm>
#include <cassert>

template<size_t N, size_t M, typename T>
struct VectorField {
    std::array<T, deltas.size()> v[N][M];

    T& add(int x, int y, int dx, int dy, T dv) {
        return get(x, y, dx, dy) += dv;
    }

    T& get(int x, int y, int dx, int dy) {
        size_t i = std::ranges::find(deltas, std::make_pair(dx, dy)) - deltas.begin();
        assert(i < deltas.size());
        return v[x][y][i];
    }
};
