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

#include "ace_intrin.h"

static u64 rkiss_param[4];

/* Bob Jenkin's PRNG based off RNG-Kiss-family
   (http://chessprogramming.wikispaces.com/Bob+Jenkins) */

#define rkiss_rotate(x, k) ((x << k) | (x >> (64 - k)))

static u64 rkiss_rand()
{
	const u64 e = rkiss_param[0] - rkiss_rotate(rkiss_param[1], 7);

	rkiss_param[0] = rkiss_param[1] ^ rkiss_rotate(rkiss_param[2], 13);
	rkiss_param[1] = rkiss_param[2] + rkiss_rotate(rkiss_param[3], 37);
	rkiss_param[2] = rkiss_param[3] + e;

	return rkiss_param[3] = (e + rkiss_param[0]);
}

static void rkiss_seed(const u32 seed)
{
	int i;

	rkiss_param[0] = 0xf1ea5eed;

	/* stockfish uses 0xd4e12c77 here */
	rkiss_param[1] = rkiss_param[2] = rkiss_param[3] = seed;

	for (i = 0; i < 39; i++) rkiss_rand();
}
