#pragma once

#include "FixedInner.hpp"

template<size_t X>
struct intX_t;

template<> struct intX_t<8>  { using type = int8_t;  };
template<> struct intX_t<16> { using type = int16_t; };
template<> struct intX_t<32> { using type = int32_t; };
template<> struct intX_t<64> { using type = int64_t; };

template<size_t N, size_t K>
using Fixed = FixedInner<typename intX_t<N>::type, K>;
