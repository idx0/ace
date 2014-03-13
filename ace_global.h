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

#include "ace_intrin.h"

/* This file defines global, precomputed bitboards and other values.  We are
   not concerned with memory here */

/* These bitboard arrays hold all valid move positions for the respective
   piece at the board position given by the array index */
u64 knight_movelist[64];
u64 bishop_movelist[64];
u64 rook_movelist[64];
u64 queen_movelist[64];
u64 king_movelist[64];
/* Pawns are different because their move rules are dependent on their color
   and whether they are capturing a piece.  We will also add double moves from
   starting positions */
u64 pawn_movelist[2][64];
u64 pawn_capturelist[2][64];
/* This bitboard has a bit set if the relative index in pawn_movelist should
   have an en passant square set */
u64 pawn_enpas[2];
/* These bitboards are the "rays" used to move rooks, bishops, and queens.
   When generating the actual moves these pieces can make, we draw a ray in
   each direction from the piece and cutoff the ray if it intersects another
   piece on the board */
u64 ray_topleft[64];
u64 ray_top[64];
u64 ray_topright[64];
u64 ray_right[64];
u64 ray_bottomright[64];
u64 ray_bottom[64];
u64 ray_bottomleft[64];
u64 ray_left[64];

extern void init_movelists();
