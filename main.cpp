#include "Fluid.hpp"
#include "Fixed.hpp"
#include "FastFixed.hpp"
#include "argv_parse.hpp"
#include "type_list.hpp"
#include "type_marker.hpp"
#include "type_utils.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

struct real_main {
    template<typename P_TYPE, typename V_TYPE, typename V_FLOW_TYPE>
    static void run() {
        std::cout << "Using following types:\n"
            << "p-type:      " << get_type_name<P_TYPE>() << "\n"
            << "v-type:      " << get_type_name<V_TYPE>() << "\n"
            << "v-flow-type: " << get_type_name<V_FLOW_TYPE>() << "\n"
            << std::endl;

        // TODO: use types
        Fluid<14, 5, Fixed<32, 16>> fluid("data.in");
        fluid.run();
    }
};

std::string to_lower_rm_space(std::string_view s) {
    std::string ans;
    ans.reserve(s.size());
    for (char c : s) {
        if (isspace(c)) {
            continue;;
        }
        ans.push_back(tolower(c));
    }
    return ans;
}

struct double_type_marker {
    using type = double;

    bool matches(std::string_view s) const {
        return to_lower_rm_space(s) == "double";
    }  
};

struct float_type_marker {
    using type = float;

    bool matches(std::string_view s) const {
        return to_lower_rm_space(s) == "float";
    }  
};

template<size_t N, size_t K>
struct fixed_type_marker {
    using type = Fixed<N, K>;

    bool matches(std::string_view s) const {
        std::stringstream ss;
        ss << "fixed(" << N << "," << K << ")";
        return to_lower_rm_space(s) == ss.str();
    }  
};

template<size_t N, size_t K>
struct fast_fixed_type_marker {
    using type = FastFixed<N, K>;

    bool matches(std::string_view s) const {
        std::stringstream ss;
        ss << "fast_fixed(" << N << "," << K << ")";
        return to_lower_rm_space(s) == ss.str();
    }  
};

#define FLOAT float_type_marker
#define DOUBLE double_type_marker
#define FIXED(N, K) fixed_type_marker<N, K>
#define FAST_FIXED(N, K) fast_fixed_type_marker<N, K>

int main(int argc, char** argv) {
    auto opts = argv_parse(argv);

    if (opts.positional.size() != 1) {
        throw std::runtime_error("exactly one positional argument expected");
    }

    auto p_type_str = opts.get_opt_or_throw("p-type");
    auto v_type_str = opts.get_opt_or_throw("v-type");
    auto v_flow_type_str = opts.get_opt_or_throw("v-flow-type");
    auto filename = opts.positional.front();

    using types = type_list<TYPES>;
    using types_product = product<types, types, types>::type;

    bool impl_found = run_for_matching<real_main, types_product>{}({p_type_str, v_type_str, v_flow_type_str});

    if (!impl_found) {
        std::cerr << "Error: Suitable types were not compiled" << std::endl;
    }
}
