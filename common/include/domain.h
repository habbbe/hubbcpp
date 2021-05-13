#include <parsimon/parsimon.h>

struct action {
    std::string name, command;
};
struct info {
    unsigned update_rate;
    std::string name, command;
};

struct separator {};
struct space {};

struct syntax_error {
    std::string error;
};

using entry = std::variant<action, info, separator, space, syntax_error>;
