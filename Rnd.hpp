#pragma once

#include <random>

class Rnd {
    public:
        template<typename T>
        static T random01() {
            return T(std::uniform_real_distribution<>{0, 1}(rnd));
        }

    private:
        static std::mt19937 rnd;
};
