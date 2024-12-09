#include "argv_parse.hpp"

#include <cstring>
#include <stdexcept>
#include <sstream>

ArgvParseResult argv_parse(char** argv) {
    std::vector<std::string> positional;
    std::unordered_map<std::string, std::string> named;

    for (char** arg_ = argv + 1; *arg_ != nullptr; ++arg_) {
        std::string arg = *arg_;
        if (arg.starts_with("--")) {
            arg.erase(arg.begin(), arg.begin() + 2);
            auto eq_pos = arg.find('=');
            if (eq_pos == std::string::npos) {
                throw std::runtime_error("equals sign (=) not found in argument");
            }
            std::string name(arg.begin(), arg.begin() + eq_pos);
            std::string value(arg.begin() + eq_pos + 1, arg.end());

            if (named.contains(name)) {
                throw std::runtime_error("repetition of argument");
            }
            named.insert({std::move(name), std::move(value)});
        } else {
            positional.push_back(arg);
        }
    }
    return ArgvParseResult(positional, named);
}

const std::string& ArgvParseResult::get_opt_or_throw(const std::string opt_name) {
    if (!named.contains(opt_name)) {
        std::stringstream ss;
        ss << "option " << opt_name << " not set";
        throw std::runtime_error(ss.str());
    }
    return named.at(opt_name);
}
