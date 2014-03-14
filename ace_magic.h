/*
 * ACE - Another Chess Engine
 * Copyright (C) 2007 Pradyumna Kannan
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
 * 
 * ---
 * This files consists of a cleaned-up version of Pradu's original magic move
 * generation code.  An original copy of this code can be downloaded from
 * http://www.pradu.us/old/Nov27_2008/Buzz/.
 */

#pragma once

#include "ace_intrin.h"

#define MINIMAL_B_BITS_SHIFT(square) 55
#define MINIMAL_R_BITS_SHIFT(square) 52

extern const unsigned int rook_shift[64];
extern const u64 rook_magics[64];
extern const u64 rook_mask[64];
extern const unsigned int bishop_shift[64];
extern const u64 bishop_magics[64];
extern const u64 bishop_mask[64];
extern u64 magicmovesbdb[5248];
extern const u64* bishop_indices[64];
extern u64 magicmovesrdb[102400];
extern const u64* rook_indices[64];

#define magic_bishop(sq, occ) *(bishop_indices[sq] + ((((occ) & bishop_mask[sq]) * bishop_magics[sq]) >> bishop_shift[sq]))
#define magic_rook(sq, occ) *(rook_indices[sq] + ((((occ) & rook_mask[sq]) * rook_magics[sq]) >> rook_shift[sq]))
#define magic_bishop_nomask(sq, occ) *(bishop_indices[sq] + (((occ) * bishop_magics[sq]) >> bishop_shift[sq]))
#define magic_rook_nomask(sq, occ) *(rook_indices[sq] + (((occ) * rook_magics[sq]) >> rook_shift[sq]))
#define magic_queen(sq, occ) (magic_bishop(sq,occ) | magic_rook(sq,occ))
#define magic_queen_nomask(sq, occ) (magic_bishop_nomask(sq,occ) | magic_rook_nomask(sq,occ))

extern void init_magic();
