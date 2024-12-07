#pragma once

#include "Fixed.hpp"

#include <random>

class Rnd {
    public:
        static Fixed random01();

    private:
        static std::mt19937 rnd;
};
