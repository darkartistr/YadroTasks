#include <stdio.h>
#include <type_traits>

template <typename T, T v>
struct num {
    static constexpr T value = v;
    using type = T;
};

struct op_add{};
struct op_sub{};
struct op_mul{};
struct op_div{};
struct op_neg{};

template <typename Op>
struct priority{};

template<>
struct priority<op_add>{ static constexpr int val = 1; };

template<>
struct priority<op_sub>{ static constexpr int val = 1; };

template<>
struct priority<op_mul>{ static constexpr int val = 2; };

template<>
struct priority<op_div>{ static constexpr int val = 2; };

template<>
struct priority<op_neg>{ static constexpr int val = 3; };

template <typename Left, typename Op, typename Right>
struct applybinary;

template <typename T1, T1 num1, typename T2, T2 num2>
struct applybinary<num<T1, num1>, op_add, num<T2, num2>> {
    using type = num<decltype(num1 + num2), num1 + num2>;
};

template <typename T1, T1 num1, typename T2, T2 num2>
struct applybinary<num<T1, num1>, op_sub, num<T2, num2>> {
    using type = num<decltype(num1 - num2), num1 - num2>;
};

template <typename T1, T1 num1, typename T2, T2 num2>
struct applybinary<num<T1, num1>, op_mul, num<T2, num2>> {
    using type = num<decltype(num1 * num2), num1 * num2>;
};

template <typename T1, T1 num1, typename T2, T2 num2>
struct applybinary<num<T1, num1>, op_div, num<T2, num2>> {
    static_assert(num2 != 0, "divide by zero");
    using type = num<decltype(num1 / num2), num1 / num2>;
};

template <typename Op, typename R>
struct applyUnary;

template <typename T1, T1 num1>
struct applyUnary<op_neg, num<T1, num1>> {
    using type = num<decltype(num1 * -1), num1 * -1>;
};

template <typename... Args>
struct Stack{};

template <typename T>
struct is_num : std::false_type {};

template <typename T, T v>
struct is_num<num<T, v>> : std::true_type {};

template <typename ValueStack, typename Op>
struct apply_step{};

template <typename R, typename L, typename... Tail, typename Op>
struct apply_step<Stack<R, L, Tail...>, Op> {
    using result = typename applybinary<L, Op, R>::type;
    using new_stack = Stack<result, Tail...>;
};

template <typename R, typename... Tail>
struct apply_step<Stack<R, Tail...>, op_neg> {
    using result = typename applyUnary<op_neg, R>::type;
    using new_stack = Stack<result, Tail...>;
};

template <typename Input, typename Values, typename Ops>
struct Process;

template <bool ShouldEval, typename Input, typename Values, typename Op>
struct Decision{};

template <typename NewOp, typename... ITail, typename... Vs, typename TopOp, typename... OTail>
struct Decision<true, Stack<NewOp, ITail...>, Stack<Vs...>, Stack<TopOp, OTail...>> {
    using Step = apply_step<Stack<Vs...>, TopOp>;
    using type = typename Process<Stack<NewOp, ITail...>, typename Step::new_stack, Stack<OTail...>>::type;
};

template <typename NewOp, typename... ITail, typename... Vs, typename... Os>
struct Decision<false, Stack<NewOp, ITail...>, Stack<Vs...>, Stack<Os...>> {
    using type = typename Process<Stack<ITail...>, Stack<Vs...>, Stack<NewOp, Os...>>::type;
};

template <typename T, T v, typename... Tail, typename... Vs, typename... Op>
struct Process<Stack<num<T, v>, Tail...>, Stack<Vs...>, Stack<Op...>> {
    using type = typename Process<Stack<Tail...>, Stack<num<T, v>, Vs...>, Stack<Op...>>::type;
};

template <typename NewOp, typename... Tail, typename... Vs>
requires (!is_num<NewOp>::value)
struct Process<Stack<NewOp, Tail...>, Stack<Vs...>, Stack<>> {
    using type = typename Process<Stack<Tail...>, Stack<Vs...>, Stack<NewOp>>::type;
};

template <typename NewOp, typename... Tail, typename... Vs, typename TopOp, typename... OTail>
requires (!is_num<NewOp>::value)
struct Process<Stack<NewOp, Tail...>, Stack<Vs...>, Stack<TopOp, OTail...>> {
    static constexpr bool should_eval = (priority<TopOp>::val >= priority<NewOp>::val);
    using type = typename Decision<should_eval, Stack<NewOp, Tail...>, Stack<Vs...>, Stack<TopOp, OTail...>>::type;
};

template <typename... Vs, typename TopOp, typename... OTail>
struct Process<Stack<>, Stack<Vs...>, Stack<TopOp, OTail...>> {
    using Step = apply_step<Stack<Vs...>, TopOp>;
    using type = typename Process<Stack<>, typename Step::new_stack, Stack<OTail...>>::type;
};

template <typename FinalNum>
struct Process<Stack<>, Stack<FinalNum>, Stack<>> {
    using type = FinalNum;
};

template <typename... Tokens>
struct expr {
    static constexpr auto eval() {
        using result_type = typename Process<Stack<Tokens...>, Stack<>, Stack<>>::type;
        return result_type::value;
    }
};

int main() {
    constexpr auto res = expr<num<int, 2>, op_add, num<int, 2>, op_mul, num<int, 2>>::eval();
    printf("%d\n", (int)res);
}