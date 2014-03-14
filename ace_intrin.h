/*
 * ACE - Another Chess Engine
 * Copyright (C) 2014  Stephen Schweizer <code@theindexzero.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "ace_defs.h"

#define upper(x) (((x) >> 32) & 0xffffffff)
#define lower(x) ((x) & 0xffffffff)

#ifdef __clang__
 /* TODO */
#elif defined (__GNUC__) || defined (__MINGW32__)
 /* GCC or Equivalent */
 typedef uint64_t u64;
 typedef uint32_t u32;
 typedef uint16_t u16;
# define ACE_MSB64(x) ((u32)__builtin_clzll(x))
# define ACE_LSB64(x) ((u32)__builtin_ctzll(x))
# define ACE_POPCNT64(x) ((u32)__builtin_popcountll(x))
# define ACE_POPCNT32(x) ((u32)__builtin_popcount(x))
#elif defined (__INTEL_COMPILER) || defined (__ICC)
 /* TODO */
#elif defined (_MSC_VER)
 /* MSVC */
 typedef unsigned __int64 u64;
 typedef unsigned __int32 u32;
 typedef unsigned __int16 u16;
# ifdef ACE_X86
#  ifdef ACE_WIN64
#   pragma intrinsic(_BitScanForward64)
#   pragma intrinsic(_BitScanReverse64)
    static u32 internal_lsb(u64 x)
    {
        unsigned long i;
        _BitScanForward64(&i, x);
        return (u32)i;
    }
#   define ACE_LSB64(x) (internal_lsb(x))
    static u32 internal_msb(u64 x)
    {
        unsigned long i;
        _BitScanReverse64(&i, x);
        return (u32)i;
    }
#   define ACE_MSB64(x) (internal_msb(x))
#   define ACE_POPCNT64(x) (__popcnt64(x))
#  else
#   pragma intrinsic(_BitScanForward)
#   pragma intrinsic(_BitScanReverse)
    static u32 internal_popcount64(u64 x)
    {
        return __popcnt((u32)upper(x)) + __popcnt((u32)lower(x));
    }
#   define ACE_POPCNT64(x) (internal_popcount64(x))
    static u32 internal_lsb(u64 x)
    {
        unsigned long i;
        if (!_BitScanForward(&i, (u32)lower(x)))
            if (_BitScanForward(&i, (u32)upper(x))) i += 32;

        return i;
    }
#   define ACE_LSB64(x) (internal_lsb(x))
    static u32 internal_msb(u64 x)
    {
        unsigned long i;
        if (_BitScanReverse(&i, (u32)upper(x)))
            i += 32;
        else
            _BitScanReverse(&i, (u32)lower(x));
        return i;
    }
#   define ACE_MSB64(x) (internal_msb(x))
#  endif
#  define ACE_POPCNT32(x) (__popcnt(x))
# else
# endif
#else
#endif

typedef unsigned char u8;
