#include <cassert>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cctype>
#include <string_view>

constexpr int parse_int(std::string_view sv) {
    if (sv.empty()) throw std::invalid_argument("empty string");
    int result = 0;
    bool negative = false;
    if (sv[0] == '-') {
        negative = true;
    }
    for (size_t i = (negative ? 1 : 0); i < sv.size(); ++i) {
        if (sv[i] < '0' || sv[i] > '9') throw std::invalid_argument("invalid character");
        result = result * 10 + (sv[i] - '0');
    }
    return negative ? -result : result;
}

std::vector<int> operator""_vi(const char* str, size_t len) {
    std::string_view sv(str, len);
    std::vector<int> result;
    size_t start = 0;
    for (size_t i = 0; i < len;) {
        while (i < len && !std::isdigit(static_cast<unsigned char>(sv[i])) && sv[i] != '-') {
            i++;
        }
        if (i >= len) break;
        size_t start = i;
        i++;
        while (i < len && std::isdigit(static_cast<unsigned char>(sv[i]))) {
            i++;
        }
        result.push_back(parse_int(sv.substr(start, i - start)));
    }
    return result;
}

int main() {
    auto const xs = "10, 20, 30, 40"_vi;
    for (int x : xs) {
        std::cout << x << " ";
    }

    std::cout << std::endl;

    return 0;
}

