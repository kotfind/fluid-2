#pragma once

#include "Fixed.hpp"

#include <array>
#include <limits>

constexpr std::array<std::pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

static constexpr Fixed inf = Fixed::from_raw(std::numeric_limits<int32_t>::max());
static constexpr Fixed eps = Fixed::from_raw(deltas.size());

constexpr size_t T = 1'000'000;

constexpr size_t N = 14, M = 5;
// constexpr size_t N = 36, M = 84;
