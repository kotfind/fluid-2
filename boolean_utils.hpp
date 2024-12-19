#pragma once

#include <cstdlib>

namespace boolean {
    constexpr double eps = 1e-3;

    template<typename T, typename U>
    bool eq(T lhs, U rhs) {
        return is_zero(lhs - rhs);
    }

    template<typename T>
    bool is_zero(T v) {
        return std::abs(v) < eps;
    }
}
