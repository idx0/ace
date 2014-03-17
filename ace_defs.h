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
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <intrin.h>
#elif defined (_WIN32)
# define ACE_WIN32
# define ACE_WINDOWS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
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

#if defined (__amd64__) || defined (_M_X64)
# define ACE_AMD
#elif defined (__i386__) || defined (_M_IX86) || defined (_X86_)
# define ACE_X86
#else
# define ACE_UNSUPPORTED
#endif

#undef ACE_POPCNT32
#undef ACE_POPCNT64
#undef ACE_LSB64
#undef ACE_MSB64

#undef ACE_BITTEST
#define ACE_BITTEST(a, b) ((a) & (1ULL << (b)))

#define ACE_NAME	"ace"
#define ACE_DESC	"Another Chess Engine"
#define ACE_AUTHOR	"Stephen Schweizer"
#define ACE_EMAIL	"<code@theindexzero.com>"
#define ACE_VERSION	"0.650"
