#include "Rnd.hpp"

Fixed Rnd::random01() {
    return Fixed::from_raw((rnd() & ((1 << 16) - 1)));
}

std::mt19937 Rnd::rnd{1337};
