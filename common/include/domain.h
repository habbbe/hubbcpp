#include <parsimon/parsimon.h>

struct action {
    std::string name, command;
    action(std::string_view name, std::string_view command) : name{name}, command{command} {}
};
struct info {
    std::string name, command;
    info(std::string_view name, std::string_view command) : name{name}, command{command} {}
};

struct separator {};
struct space {};

struct syntax_error {
    std::string error;
    syntax_error(std::string_view error) : error{error} {}
};

using entry = std::variant<action, info, separator, space, syntax_error>;
