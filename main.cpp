#include "Fluid.hpp"
#include "Fixed.hpp"

int main() {
    Fluid<14, 5, Fixed> fluid;
    fluid.run();
}
