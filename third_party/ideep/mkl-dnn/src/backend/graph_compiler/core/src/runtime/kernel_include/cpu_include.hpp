/*******************************************************************************
 * Copyright 2020-2022 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_RUNTIME_KERNEL_INCLUDE_CPU_INCLUDE_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_RUNTIME_KERNEL_INCLUDE_CPU_INCLUDE_HPP

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <immintrin.h>
#include <stdint.h>
#ifdef __AVX512F__
#include "x86simd/vec_f32x16.hpp"
#include "x86simd/vec_s32x16.hpp"
#include "x86simd/vec_s8x64.hpp"
#include "x86simd/vec_u16x32.hpp"
#include "x86simd/vec_u32x16.hpp"
#include "x86simd/vec_u8x64.hpp"
#endif

#ifdef __AVX2__
#include "x86simd/vec_f32x8.hpp"
#include "x86simd/vec_s32x8.hpp"
#include "x86simd/vec_s8x32.hpp"
#include "x86simd/vec_u16x16.hpp"
#include "x86simd/vec_u32x8.hpp"
#include "x86simd/vec_u8x32.hpp"
#endif

#ifdef __SSE3__
#include "x86simd/vec_f32x4.hpp"
#include "x86simd/vec_s32x4.hpp"
#include "x86simd/vec_s8x16.hpp"
#include "x86simd/vec_u16x4.hpp"
#include "x86simd/vec_u16x8.hpp"
#include "x86simd/vec_u32x4.hpp"
#include "x86simd/vec_u8x16.hpp"
#endif

#include <runtime/generic_val.hpp>

namespace sc {
namespace runtime {
struct stream_t;
}
} // namespace sc

#ifndef SC_JIT_SOURCE // if it is compiled by SC-gtests
extern "C" void *sc_aligned_malloc(
        sc::runtime::stream_t *stream, size_t sz) noexcept;
extern "C" void sc_aligned_free(
        sc::runtime::stream_t *stream, void *p) noexcept;
extern "C" void *sc_thread_aligned_malloc(
        sc::runtime::stream_t *stream, size_t sz) noexcept;
extern "C" void sc_thread_aligned_free(
        sc::runtime::stream_t *stream, void *p) noexcept;
extern "C" void *sc_global_aligned_alloc(size_t sz, size_t align) noexcept;
extern "C" void sc_global_aligned_free(void *ptr, size_t align) noexcept;
#endif

#define DEF_MINMAX(T) \
    inline T sc_min(T v1, T v2) { return v1 < v2 ? v1 : v2; } \
    inline T sc_max(T v1, T v2) { return v1 > v2 ? v1 : v2; }
#define DEF_OP(T) \
    DEF_MINMAX(T) \
    inline T sc_abs(T v1) { return v1 > 0 ? v1 : -v1; }
#define DEF_ROUND(T) \
    inline T sc_floor(T v1) { return floor(v1); } \
    inline T sc_ceil(T v1) { return ceil(v1); }
#define DEF_EXP(T) \
    inline T sc_exp(T v1) { return expf(v1); }
#define DEF_SQRT(T) \
    inline T sc_sqrt(T v1) { return std::sqrt(v1); }
#define DEF_RSQRT(T) \
    inline T sc_rsqrt(T v1) { return 1.0 / std::sqrt(v1); }
#define inf INFINITY

#define DEF_FMADD(T) \
    inline T sc_fmadd(T v1, T v2, T v3) { return v1 * v2 + v3; }
DEF_OP(float)
DEF_OP(int32_t)
DEF_OP(int8_t)
DEF_MINMAX(uint8_t)
DEF_MINMAX(uint16_t)
DEF_MINMAX(uint32_t)
DEF_MINMAX(uint64_t)
DEF_ROUND(float)
DEF_EXP(float)
DEF_SQRT(float)
DEF_RSQRT(float)
DEF_FMADD(float)

using generic_val = sc::generic_val;

template <typename T, typename T2>
T sc_reinterpret(T2 v) {
    union {
        T v1;
        T2 v2;
    } val;
    val.v2 = v;
    return val.v1;
}

inline float sc_round(float v) {
    __m128 sseval = _mm_set_ss(v);
    __m128 rounded_val = _mm_round_ps(sseval, _MM_FROUND_TO_NEAREST_INT);
    return _mm_cvtss_f32(rounded_val);
}

inline bool sc_isnan(float v) {
    return std::isnan(v);
}

inline bool sc_isnan(uint16_t v) {
    return (v & 0x7F80) == 0x7F80 && (v & 0x007F);
}

template <class T1, class T2>
inline T1 sc_saturated_cast(const T2 &v) {
    return static_cast<T1>(v);
}

template <>
inline int8_t sc_saturated_cast(const int32_t &v) {
    if (v > 127) {
        return 127;
    } else if (v < -128) {
        return -128;
    } else {
        return v;
    }
}
template <>
inline uint8_t sc_saturated_cast(const int32_t &v) {
    if (v > 255) {
        return 255;
    } else if (v < 0) {
        return 0;
    } else {
        return v;
    }
}
template <>
inline int8_t sc_saturated_cast(const float &v) {
    int iv = std::round(v);
    if (iv > 127) {
        return 127;
    } else if (iv < -128) {
        return -128;
    } else {
        return (int8_t)(iv);
    }
}
template <>
inline uint8_t sc_saturated_cast(const float &v) {
    int iv = std::round(v);
    if (iv > 255) {
        return 255;
    } else if (iv < 0) {
        return 0;
    } else {
        return (uint8_t)iv;
    }
}

template <class T1, class T2>
inline T1 sc_round_and_cast(const T2 &v) {
    return static_cast<T1>(v);
}

template <>
inline int32_t sc_round_and_cast(const float &v) {
    return std::roundf(v);
}

#ifdef __AVX512F__
INLINE vec_f32x16 sc_permutex2var(
        vec_f32x16 const &a, vec_u32x16 const &idx, vec_f32x16 const &b) {
    return _mm512_permutex2var_ps(a.v, idx.v, b.v);
}
INLINE vec_f32x4 sc_permutex2var(
        vec_f32x4 const &a, vec_u32x4 const &idx, vec_f32x4 const &b) {
    return _mm_permutex2var_ps(a.v, idx.v, b.v);
}
#endif

#include "x86simd/vector_cast.hpp"
#include "x86simd/vector_maskloadstore.hpp"

#endif
