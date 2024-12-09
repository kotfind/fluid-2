#pragma once

#include <type_traits>

namespace type_list {
    /// list
    /// A list of types. Is written via [...] in further comments:
    /// list<A, B, C> is [A, B, C]
    template<typename... Ts>
    struct list {};

    /// is_list<T>
    /// Checks if T is list.
    template<typename T>
    struct is_list : std::false_type {};

    template<typename... Ts>
    struct is_list<list<Ts...>> : std::true_type {};

    /// push_front<T, [Ts...]>
    /// Pushed T to list.
    /// Example:
    ///     push_front<A, [B, C]> -> [A, B, C]
    template<typename... Ts>
    struct push_front;

    template<typename T, typename... Ts>
    struct push_front<T, list<Ts...>> {
        using type = list<T, Ts...>;
    };

    /// push_front_for_each<T, [[Ts...]...]>
    /// Pushes T to each item in the list of lists.
    /// Example:
    ///     push_front_for_each<A, [[B, C], [D, E]]> -> [[A, B, C], [A, D, E]]
    template<typename... Ts>
    struct push_front_for_each;

    template<typename T, typename... Ls>
    requires std::conjunction<is_list<Ls>...>::value
    struct push_front_for_each<T, list<Ls...>> {
        using type = list<typename push_front<T, Ls>::type...>;
    };

    /// concat<[Ts...]...>
    /// Concatenates N lists.
    /// Example:
    ///     concat<[A, B], [C, D], [E, F]> -> [A, B, C, D, E, F]
    template<typename... Ts>
    struct concat;

    template<typename... Ts, typename... Ls>
    struct concat<list<Ts...>, Ls...> {
        using type = typename concat<list<Ts...>, typename concat<Ls...>::type>::type;
    };

    template<typename... Ts, typename... Us>
    struct concat<list<Ts...>, list<Us...>> {
        using type = list<Ts..., Us...>;
    };

    template<typename... Ts>
    struct concat<list<Ts...>> {
        using type = list<Ts...>;
    };

    template<>
    struct concat<> {
        using type = list<>;
    };

    /// product<[Ts...]...>
    /// Creates cartesian product of N lists.
    /// Example:
    ///     product<[A, B], [C, D], [E, F]> -> [
    ///         [A, C, E],
    ///         [A, C, F],
    ///         [A, D, E],
    ///         [A, D, F],
    ///         [B, C, E],
    ///         [B, C, F],
    ///         [B, D, E],
    ///         [B, D, F],
    ///     ]
    template<typename... Ts>
    struct product;

    template<typename... Ts, typename... Ls>
    requires std::conjunction<is_list<Ls>...>::value
    struct product<list<Ts...>, Ls...> {
        using type = typename concat<
                typename push_front_for_each<
                    Ts,
                    typename product<Ls...>::type
                >::type...
            >::type;
    };

    template<typename... Ts>
    struct product<list<Ts...>> {
        using type = list<list<Ts>...>;
    };

    template<>
    struct product<> {
        using type = list<>;
    };
};
