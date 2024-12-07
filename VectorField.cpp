#include "VectorField.hpp"

#include <cassert>

Fixed& VectorField::add(int x, int y, int dx, int dy, Fixed dv) {
    return get(x, y, dx, dy) += dv;
}

Fixed& VectorField::get(int x, int y, int dx, int dy) {
    size_t i = std::ranges::find(deltas, std::make_pair(dx, dy)) - deltas.begin();
    assert(i < deltas.size());
    return v[x][y][i];
}
