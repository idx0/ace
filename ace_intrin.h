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
 
#define C64(x) (x##ULL)

#if defined (__GNUC__)
  /* GCC or Equivalent */
# define ALIGN64 __attribute__((aligned(8)))
# define ALIGN32 __attribute__((aligned(4)))
# define FORCE_INLINE __attribute__((always_inline))
  typedef uint64_t u64;
  typedef uint32_t u32;
  typedef uint16_t u16;
  typedef int32_t  s32;
  typedef int16_t  s16;
# define ACE_MSB64(x) (63 - (u32)__builtin_clzll(x))
# define ACE_LSB64(x) ((u32)__builtin_ctzll(x))
# define ACE_POPCNT64(x) ((u32)__builtin_popcountll(x))
# define ACE_POPCNT32(x) ((u32)__builtin_popcount(x))
#elif defined (__INTEL_COMPILER) || defined (__ICC)
  /* TODO */
#elif defined (_MSC_VER)
  /* MSVC */
# define ALIGN64 __declspec(align(8))
# define ALIGN32 __declspec(align(4))
# define FORCE_INLINE __forceinline
  typedef unsigned __int64 u64;
  typedef unsigned __int32 u32;
  typedef unsigned __int16 u16;
  typedef __int32          s32;
  typedef __int16          s16;
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
# define FORCE_INLINE inline
# define ALIGN64
#endif

typedef unsigned char u8;

#include <stdlib.h>

/* aligned malloc/free */
extern void *xmalloc(size_t sz, size_t align);
extern void xfree(void *ptr);

#ifdef ACE_WINDOWS
typedef struct ms_time {
  u64 time;
} ms_time_t;
#else
#include <sys/time.h>
typedef struct ms_time {
  struct timeval time;
} ms_time_t;
#endif

extern void get_current_tick(ms_time_t* t);

extern u64 get_interval(const ms_time_t *then, const ms_time_t* now);

/* reentrant strtok (also the one that makes MSVC happy) */
#ifdef ACE_WINDOWS
# define strtok2(str, delim, ctx) (strtok_s(str, delim, ctx))
# define sscanf(str, fmt, ...) (sscanf_s(str, fmt, __VA_ARGS__))
#else
# if defined(_SVID_SOURCE) || defined(_BSD_SOURCE) || defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE) || defined(_POSIX_SOURCE)
#  define strtok2(str, delim, ctx) (strtok_r(str, delim, ctx))
# else
#  define strtok2(str, delim, ctx) (strtok(str, delim))
# endif
#endif
