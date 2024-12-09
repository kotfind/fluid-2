#pragma once

#include "Matrix.hpp"
#include "const.hpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <memory>

template<typename T>
struct VectorField {
    std::unique_ptr<AbstractMatrix<std::array<T, deltas.size()>>> v = nullptr;

    VectorField()
      : v(nullptr)
    {}

    VectorField(size_t n, size_t m)
      : v(create_matrix<std::array<T, deltas.size()>>{}(n, m))
    {}

    void reset() {
        v->reset();
    }

    T& add(int x, int y, int dx, int dy, T dv) {
        assert(v.get() != nullptr);
        return get(x, y, dx, dy) += dv;
    }

    T& get(int x, int y, int dx, int dy) {
        assert(v.get() != nullptr);
        size_t i = std::ranges::find(deltas, std::make_pair(dx, dy)) - deltas.begin();
        assert(i < deltas.size());
        return (*v)[x][y][i];
    }
};
