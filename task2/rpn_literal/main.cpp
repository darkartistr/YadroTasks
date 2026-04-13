#include <cassert>
#include <vector>
#include <stdexcept>
#include <string_view>

constexpr int parse_int(std::string_view sv) {
    int result = 0;
    bool negative = false;
    if (sv[0] == '-') {
        negative = true;
    }
    for (size_t i = (negative ? 1 : 0); i < sv.size(); ++i) {
        if (sv[i] < '0' || sv[i] > '9') throw std::invalid_argument("Invalid character");
        result = result * 10 + (sv[i] - '0');
    }
    return negative ? -result : result;
}

constexpr int pow_int(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; ++i) result *= base;
    return result;
}

constexpr int apply_op(int a, int b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) throw std::invalid_argument("Division by zero");
            return a / b;
        case '%': return a % b;
        case '^': return pow_int(a, b);
        default:  return 0;
    }
}

constexpr int operator""_rpn(const char* str, size_t len) {
    std::string_view sv(str, len);
    std::vector<int> stack;
    size_t start = 0;
    for (size_t i = 0; i <= len; ++i) {
        if (i == len || sv[i] == ' ' || sv[i] == ',') {
            if (i > start) {
                std::string_view token = sv.substr(start, i - start);
                if (token.size() == 1 && (token[0] < '0' || token[0] > '9')) {
                    if (stack.size() >= 2) {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(apply_op(a, b, token[0]));
                    } else { throw std::runtime_error("Stack underflow"); }
                    }
                else { stack.push_back(parse_int(token)); }
            }
            start = i + 1;
        }
    }
    if (stack.size() != 1) throw std::runtime_error("Invalid expression");
    return stack.back();
}

int main() {

    assert(("2 3 +"_rpn) == 2 + 3);
    assert(("2 3 + 4 5 + *"_rpn) == (2 + 3) * (4 + 5));
    assert(("10 3 /"_rpn) == 10 / 3);
    assert(("2 3 ^"_rpn) == 2 * 2 * 2);
    assert(("5 1 2 + 4 * + 3 -"_rpn) == 5 + (1 + 2) * 4 - 3);

    return 0;
}
