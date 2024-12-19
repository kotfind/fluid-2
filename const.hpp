#pragma once

#include <array>
#include <cassert>
#include <cstddef>

// NOTE: don't change the order (delta_idx)
constexpr std::array<std::pair<int, int>, 4> deltas{{
    {-1,  0}, // 0
    { 1,  0}, // 1
    { 0, -1}, // 2
    { 0,  1}, // 3
}};

inline size_t delta_idx(size_t dx, size_t dy) {
    size_t i = -1;
    if (dx == 0) {
        if (dy == -1) {
            i = 2;
        } else {
            assert(dy == 1);
            i = 3;
        }
    } else {
        assert(dy == 0);
        if (dx == -1) {
            i = 0;
        } else {
            assert(dx == 1);
            i = 1;
        }
    }
    assert(i != -1);
    assert(deltas[i].first == dx);
    assert(deltas[i].second == dy);
    return i;
}
