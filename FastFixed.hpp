#pragma once

#include "FixedInner.hpp"

#include <cstdint>

template<size_t X>
struct int_fastX_t;

template<size_t X>
requires (X <= 8)
struct int_fastX_t<X> {
    #if defined INT_FAST8_MAX
        using type = int_fast8_t;
    #elif defined INT_FAST16_MAX
        using type = int_fast16_t;
    #elif defined INT_FAST32_MAX
        using type = int_fast32_t;
    #elif defined INT_FAST64_MAX
        using type = int_fast64_t;
    #else
        #error "suitable fast_int_t type does not exist"
    #endif
};

template<size_t X>
requires (8 < X && X <= 16)
struct int_fastX_t<X> {
    #if defined INT_FAST16_MAX
        using type = int_fast16_t;
    #elif defined INT_FAST32_MAX
        using type = int_fast32_t;
    #elif defined INT_FAST64_MAX
        using type = int_fast64_t;
    #else
        #error "suitable fast_int_t type does not exist"
    #endif
};

template<size_t X>
requires (16 < X && X <= 32)
struct int_fastX_t<X> {
    #if defined INT_FAST32_MAX
        using type = int_fast32_t;
    #elif defined INT_FAST64_MAX
        using type = int_fast64_t;
    #else
        #error "suitable fast_int_t type does not exist"
    #endif
};

template<size_t X>
requires (32 < X && X <= 64)
struct int_fastX_t<X> {
    #if defined INT_FAST64_MAX
        using type = int_fast64_t;
    #else
        #error "suitable fast_int_t type does not exist"
    #endif
};

template<size_t N, size_t K>
using FastFixed = FixedInner<typename int_fastX_t<N>::type, K>;
