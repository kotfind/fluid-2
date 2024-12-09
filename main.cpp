#include "Fluid.hpp"
#include "Fixed.hpp"
#include "argv_parse.hpp"

#include <stdexcept>
#include <string>

template<typename P_TYPE, typename V_TYPE, typename V_FLOW_TYPE>
void real_main() {
    // TODO: use types
    Fluid<14, 5, Fixed<32, 16>> fluid;
    fluid.run();
}

int main(int argc, char** argv) {
    auto opts = argv_parse(argv);

    if (opts.positional.size() != 1) {
        throw std::runtime_error("exactly one positional argument expected");
    }

    auto p_type_str = opts.get_opt_or_throw("p-type");
    auto v_type_str = opts.get_opt_or_throw("v-type");
    auto v_flow_type_str = opts.get_opt_or_throw("v-flow-type");
    auto filename = opts.positional.front();

    std::cout << p_type_str << '.' << v_type_str << '.' << v_flow_type_str << '.' << filename << std::endl;
}
