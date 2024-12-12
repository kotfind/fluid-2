#include <string>
#include <unordered_map>
#include <vector>

struct ArgvParseResult {
    const std::vector<std::string> positional;
    const std::unordered_map<std::string, std::string> named;

    const std::string& get(const std::string opt_name);
    const std::string* get_if(const std::string opt_name);
};

ArgvParseResult argv_parse(char** argv);
