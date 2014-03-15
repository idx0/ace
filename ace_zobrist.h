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

#include "ace_rkiss.h"

/* This file defines the variables and functions needed to perform Zobrist
   hashing of a board position.  I hash into a u64. */

/* hash of piece color and position */
u64 hash_pawns[2][64];
u64 hash_knights[2][64];
u64 hash_rooks[2][64];
u64 hash_bishops[2][64];
u64 hash_queens[2][64];
u64 hash_kings[2][64];
/* pointers for ease of use */
u64* hash_piece[2][6];
/* hash of side (xor'd if side is black) */
u64 hash_side;
/* hash of castling permissions - there are 4 castling permissions but we will
   hash them as a 4 bit mask */
u64 hash_castle[16];
/* hash of en passant file */
u64 hash_enpas[8];

extern void init_zobrist(const u32 seed);
