#pragma once

#include "type_list.hpp"

#include <stdexcept>
#include <string_view>
#include <tuple>

template<typename T>
concept is_type_marker = requires(const T& t, std::string_view s) {
    typename T::type;

    { t.matches(s) } -> std::same_as<bool>;
} && std::is_default_constructible<T>::value;

template<typename T>
struct matches_list;

template<is_type_marker... Ms>
struct matches_list<type_list<Ms...>> {
    bool operator()(const std::array<std::string_view, sizeof...(Ms)>& s) {
        return helper(
            s,
            std::make_tuple<Ms...>(Ms{}...),
            std::make_index_sequence<sizeof...(Ms)>{}
        );
    }

    private:
        template<size_t... Is>
        bool helper(
            const std::array<std::string_view, sizeof...(Ms)>& s,
            const std::tuple<Ms...>& ms,
            std::index_sequence<Is...>
        ) {
            return (get<Is>(ms).matches(s[Is]) && ...);
        }
};

template<typename T>
struct is_type_marker_list_ : std::false_type {};

template<is_type_marker... Ms>
struct is_type_marker_list_<type_list<Ms...>> : std::true_type {};

template<typename T>
concept is_type_marker_list = is_type_marker_list_<T>::value;

template<typename... Ts>
struct run_for_matching;

template<typename F, is_type_marker... Ms, is_type_marker_list... Ls>
struct run_for_matching<F, type_list<type_list<Ms...>, Ls...>> {
    void operator()(F& func, const std::array<std::string_view, sizeof...(Ms)>& types) {
        if (matches_list<type_list<Ms...>>{}(types)) {
            func.template run<typename Ms::type...>();
            return;
        }
        return run_for_matching<F, type_list<Ls...>>{}(func, types);
    }
};

template<typename F>
struct run_for_matching<F, type_list<>> {
    template<size_t N>
    void operator()(F& func, const std::array<std::string_view, N>& types) {
        throw std::runtime_error("suitable implementation was not found");
    }
};
