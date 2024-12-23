#include "Fluid.hpp"
#include "Fixed.hpp"
#include "FastFixed.hpp"
#include "argv_parse.hpp"
#include "type_list.hpp"
#include "type_marker.hpp"
#include "type_utils.hpp"

#include <cctype>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

struct real_main {
    std::string filename;
    size_t ticks_count = 1'000'000;
    bool quiet = false;

    template<typename P_TYPE, typename V_TYPE, typename V_FLOW_TYPE>
    void run() {
        std::cout << "Using following types:\n"
            << "p-type:      " << get_type_name<P_TYPE>() << "\n"
            << "v-type:      " << get_type_name<V_TYPE>() << "\n"
            << "v-flow-type: " << get_type_name<V_FLOW_TYPE>() << "\n"
            << std::endl;

        Fluid<P_TYPE, V_TYPE, V_FLOW_TYPE> fluid(filename);

        auto start_time = std::chrono::system_clock::now();
        fluid.run(ticks_count, quiet);
        auto end_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);

        std::cout << "\nExcuted in " << duration << std::endl;
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
    real_main r_main;

    auto opts = argv_parse(argv);

    auto p_type_str = opts.get("p-type");
    auto v_type_str = opts.get("v-type");
    auto v_flow_type_str = opts.get("v-flow-type");

    if (opts.positional.size() != 1) {
        throw std::runtime_error("exactly one positional argument expected");
    }
    r_main.filename = opts.positional.front();

    if (auto* ticks_count = opts.get_if("ticks")) {
        r_main.ticks_count = std::stoi(*ticks_count);
    }

    if (auto* quiet = opts.get_if("quiet")) {
        if (*quiet == "true") {
            r_main.quiet = true;
        } else if (*quiet == "false") {
            r_main.quiet = true;
        } else {
            throw std::runtime_error("either 'true' or 'false' expected");
        }
    }

    using types = type_list<TYPES>;
    using types_product = product<types, types, types>::type;

    run_for_matching<real_main, types_product>{}(
        r_main,
        {p_type_str, v_type_str, v_flow_type_str}
    );
}
