
#include <string_view>
#include <iterator>

constexpr int parse_int(std::string_view sv) {
    int result = 0;
    for (char c : sv) {
        if (c < '0' || c > '9') throw std::invalid_argument("Invalid character");
        result = result * 10 + (c - '0');
    }
    return result;
}


constexpr bool is_in_list(std::string_view  needle, std::string_view const* haystack, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (haystack[i] == needle) return true;
    }
    return false;
}


constexpr std::string_view interesting[] = {"init", "open", "read", "write", "close"};
constexpr std::size_t len = std::size(interesting);

constexpr bool is_interesting(std::string_view name) {
    return is_in_list(name, interesting, len);
}

static_assert(is_interesting("read"), "read is interesting");
static_assert(!is_interesting("get_time"), "get_time is NOT interesting");
static_assert(parse_int("123") == 123, "Should parase");
//static_assert(parse_int("123a") == 123);

int main() {

    return 0;
}
