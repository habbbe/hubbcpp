#include <fstream>
#include <cstdlib>
#include "parser.h"

constexpr auto entry_parser = []{
    using namespace parsimon;

    constexpr auto add_to_state = [](auto& s, auto&& arg) {
        s.emplace_back(std::forward<decltype(arg)>(arg));
    };

    constexpr auto parse_name = until_item('=');
    constexpr auto parse_cmd = not_empty(rest());
    constexpr auto parse_update_rate = (item('-') >> integer<unsigned>() << item(':')) || (item(':') >= 1000u);
    constexpr auto parse_action = seq("Com:") >> lift_value<action>(parse_name, parse_cmd);
    constexpr auto parse_info = seq("Info") >> lift_value<info>(parse_update_rate, parse_name, parse_cmd);
    constexpr auto parse_separator = seq("Separator") >> mreturn_emplace<separator>();
    constexpr auto parse_space = seq("Space") >> mreturn_emplace<space>();
    constexpr auto parse_error = lift_value<syntax_error>(rest());
    constexpr auto ignore = empty() || (item('#') >> rest());
    return ignore || lift_or_state(add_to_state, parse_action, parse_info, parse_separator, parse_space, parse_error);
}();

std::string default_config() {
    return std::string{getenv("HOME")} + "/.hubb";
}

std::vector<entry> parse(const std::string& path) {
    std::ifstream in(path);
    std::vector<entry> v;
    std::string line;
    while (std::getline(in, line)) {
        entry_parser.parse_with_state(line, v);
    }
    return v;
}
