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

#if defined (_WIN64)
# define ACE_WIN64
# define ACE_WINDOWS
# include <intrin.h>
#elif defined (_WIN32)
# define ACE_WIN32
# define ACE_WINDOWS
# include <intrin.h>
#elif defined (__APPLE__)
# define ACE_MAC
/* Unix is any system that runs *nix software (BSD, Linux, or OSX) */
# define ACE_UNIX
#else /* other *MUST* be linux */
# define ACE_LINUX
# define ACE_UNIX
# include <inttypes.h>
#endif

#undef ACE_BITTEST
#undef ACE_POPCNT32
#undef ACE_POPCNT64
#undef ACE_LSB64
#undef ACE_MSB64

#ifdef __clang__
 /* TODO */
#elif defined (__GNUC__) || defined (__MINGW32__)
 /* GCC or Equivalent */
 typedef uint64_t u64;
 typedef uint32_t u32;
 typedef uint16_t u16;
#elif defined (__INTEL_COMPILER) || defined (__ICC)
 /* TODO */
#elif defined (_MSC_VER)
 /* MSVC */
 typedef unsigned __int64 u64;
 typedef unsigned __int32 u32;
 typedef unsigned __int16 u16;
# ifdef ACE_WIN64
#  pragma intrinsic(_BitScanForward64)
#  pragma intrinsic(_BitScanReverse64)
#  pragma intrinsic(_bittest64)
#  define ACE_BITTEST(a, b) (_bittest64(&(a), (b)))
   u32 internal_bitScanForward(u64 x)
   {
       unsigned long i;
       _BitScanForward64(&i, x);
       return (u32)i;
   }
#  define ACE_LSB64(x) (internal_bitScanForward(x))
   u32 internal_bitScanReverse(u64 x)
   {
       unsigned long i;
       _BitScanReverse64(&i, x);
       return (u32)i;
   }
#  define ACE_MSB64(x) (internal_bitScanReverse(x))
# else
# endif
# define ACE_POPCNT32(x) (__popcnt(x))
# define ACE_POPCNT64(x) (__popcnt64(x))
#else
# define ACE_BITTEST(a, b) ((a)) & (0x1 << (b)))
#endif

typedef unsigned char u8;
