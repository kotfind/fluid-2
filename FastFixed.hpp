#pragma once

#include "FixedInner.hpp"

#include <cstdint>

template<size_t X>
struct int_fastX_t;

template<> struct int_fastX_t<8>  { using type = int_fast8_t;  };
template<> struct int_fastX_t<16> { using type = int_fast16_t; };
template<> struct int_fastX_t<32> { using type = int_fast32_t; };
template<> struct int_fastX_t<64> { using type = int_fast64_t; };

template<size_t N, size_t K>
using FastFixed = FixedInner<typename int_fastX_t<N>::type, K>;
