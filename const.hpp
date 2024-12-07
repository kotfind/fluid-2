#pragma once

#include <array>
#include <cstddef>

constexpr std::array<std::pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

constexpr size_t N = 14, M = 5;
// constexpr size_t N = 36, M = 84;
