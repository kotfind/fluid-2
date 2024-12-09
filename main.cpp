#include "Fluid.hpp"
#include "Fixed.hpp"

int main() {
    Fluid<14, 5, Fixed<32, 16>> fluid;
    fluid.run();
}
