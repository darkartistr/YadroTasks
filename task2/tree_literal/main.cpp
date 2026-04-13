#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>

struct int_tree_t {
    int value;
    std::unique_ptr<int_tree_t> left;
    std::unique_ptr<int_tree_t> right;
};

constexpr int parse_int(std::string_view sv) {
    assert(!sv.empty() && "String view passed to parse_int should not be empty");
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

using iter = std::string_view::const_iterator;

struct parse_state {
    iter pos;
    iter end;

    char peek() const { return pos != end ? *pos : '\0'; }
    void advance() { assert(pos != end && "Attempted to advance past the end of input"); ++pos; }
    void skip_whitespace() { while (pos != end && (*pos == ' ' || *pos == '\t')) ++pos; }
    void error(std::string_view msg) const { throw std::runtime_error("Parsing error: " + std::string(msg));}
};

int parse_number_token(parse_state& state) {
    state.skip_whitespace();
    iter start = state.pos;
    if (state.peek() == '-') state.advance();
    while (state.pos != state.end && std::isdigit(static_cast<unsigned char>(state.peek()))) {
        state.advance();
    }
    assert(start != state.pos && "parse_number_token logic failed to consume characters");
    return parse_int(std::string_view(&(*start), std::distance(start, state.pos)));
}

std::unique_ptr<int_tree_t> parse_node(parse_state& state) {
    state.skip_whitespace();
    if (state.peek() == '<') {
        state.advance();

        std::unique_ptr<int_tree_t> left;
        state.skip_whitespace();
        if (state.peek() != '|') {
            left = parse_node(state);
        }

        state.skip_whitespace();
        if (state.peek() != '|') state.error("expected '|' before value");
        assert(state.peek() == '|' && "Invariant violated: expected '|'");
        state.advance();

        int value = parse_number_token(state);

        state.skip_whitespace();
        if (state.peek() != '|') state.error("expected '|' after value");
        assert(state.peek() == '|' && "Invariant violated: expected '|'");
        state.advance();

        std::unique_ptr<int_tree_t> right;
        state.skip_whitespace();
        if (state.peek() != '>') {
            right = parse_node(state);
        }
        state.skip_whitespace();
        if (state.peek() != '>') state.error("expected '>'");
        assert(state.peek() == '>' && "Invariant violated: expected '>'");
        state.advance();

        auto node = std::make_unique<int_tree_t>(int_tree_t{value, std::move(left), std::move(right)});
        assert(node != nullptr);
        return node;
    } else {

        auto leaf = std::make_unique<int_tree_t>(int_tree_t{parse_number_token(state), nullptr, nullptr});
        assert(leaf != nullptr);
        return leaf;
    }
}

std::unique_ptr<int_tree_t> operator""_ti(const char* str, size_t len) {
    std::string_view sv(str, len);
    parse_state state{sv.begin(), sv.end()};
    return parse_node(state);
}

void fine_print_tree(const std::unique_ptr<int_tree_t>& node, int indent = 0) {
    if (node == nullptr) return;
    fine_print_tree(node->right, indent + 4);
    for (int i = 0; i < indent; ++i) std::cout << ' ';
    std::cout << node->value << '\n';
    fine_print_tree(node->left, indent + 4);
}

int main() {
    auto const t = "<<<<|0|>|1|<|2|>>|3|>|7|<<|9|>|11|<|13|>>>"_ti;
    fine_print_tree(t);
    return 0;
}
