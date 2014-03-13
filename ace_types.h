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

#ifndef TRUE
#define TRUE 	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

/* piece definition: piece type */
typedef enum piece_type { PAWN = 0, KNIGHT, ROOK, BISHOP, QUEEN, KING, INVALID } piece_type_t;

#define piece_valid(x) (((x) >= PAWN) && ((x) < INVALID))

/* piece definition: piece color, also used for side */
typedef enum piece_color { WHITE = 0, BLACK } piece_color_t;

/* board definition: rank row designations R1-R8 */
typedef enum board_rank { R1 = 0, R2, R3, R4, R5, R6, R7, R8 } board_rank_t;

/* board definition: file column designations FA-FH */
typedef enum board_file { FA = 0, FB, FC, FD, FE, FF, FG, FH } board_file_t;

/* side definition: castling permissions (king-side versus queen-side) */
typedef enum side_castle { WK = 1, WQ = 2, BK = 4, BQ = 8 } side_castle_t;

typedef enum algebraic {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	INVALID_MOVE
} algebraic_t;

/* side definition: color, this is a redefinition of piece_color for
   consistancy */
#define side_color_t piece_color_t

/* this structure holds the rank and file in an 8 bit word */
struct position {
	u8 rank : 4;
	u8 file : 4;
};

#define POSITION_INVALID 0x88
#define POSITION_MASK	 0x77
#define ASU8(pos) (*(u8*)&(pos))

/* this structure defines a piece, as used by our piece list */
typedef struct piece {
	/* union of position structure and u8 */
	union {
		struct position p;
		u8 				u;
	} pos;

	/* piece color */
	piece_color_t color;

	/* piece type */
	piece_type_t  type;
} piece_t;


#define rank(x) (((x) & 0x38) >> 3)
#define file(x) ((x) & 0x07)
#define is_valid_index(x) (((x) >= 0) && ((x) <= 63))
#define from_rank_file(r, f) ((((r) & 0x07) << 3) | ((f) & 0x07))
#define set(a, b) ((a) & (1ULL << (b)))

typedef struct board {

	/* bitboard which represents all pieces */
	u64 piece[2][6];

	piece_t squares[64];

	/* bitboard which represents all occupied sq for this color */
	u64 occ[2];

	/* number of moves and half-moves (half-moves are respective of 50 move
	   rule) */
	u32 plies;
	u32 moves;

	/* side color */
	side_color_t side;

	/* side castling permissions as a bit mask */
	u8 castle;
} board_t;
