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

#ifdef ACE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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

/* piece definition: piece type */
typedef enum piece_type { PAWN = 0, KNIGHT, ROOK, BISHOP, QUEEN, KING } piece_type_t;

/* piece definition: piece color, also used for side */
typedef enum piece_color { WHITE = 0, BLACK } piece_color_t;

/* board definition: rank row designations R1-R8 */
typedef enum board_rank { R1 = 0, R2, R3, R4, R5, R6, R7, R8 } board_rank_t;

/* board definition: file column designations FA-FH */
typedef enum board_file { FA = 0, FB, FC, FD, FE, FF, FG, FH } board_file_t;

/* side definition: castling permissions (king-side versus queen-side) */
typedef enum side_castle { WK = 1, WQ = 2, BK = 4, BQ = 8 } side_castle_t;

/* algebraic move notation enumeration */
typedef enum algebraic {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	INVALID_SQUARE
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
typedef u8 piece_t;

/* piece setting and getting macros */
#define piece_type(p)  ((p) & 0x07)
#define piece_color(p) (((p) & 0x08) >> 3)
#define set_piece_color(p, c) ((p) = ((p) & 0xf7) | (((c) & 0x01) << 3))
#define set_piece_type(p, t)  ((p) = ((p) & 0xf8) | ((t) & 0x07))

/* piece checking macros */
#define INVALID_PIECE	0xff
#define piece_valid(p) (((p) != INVALID_PIECE) && (piece_type(p) <= KING))

/* rank, file, index conversion macros */
#define rank(x) (((x) & 0x38) >> 3)
#define file(x) ((x) & 0x07)
#define is_valid_index(x) ((x) <= 63)
#define from_rank_file(r, f) ((((r) & 0x07) << 3) | ((f) & 0x07))

/* bittest redefinition */
#define set(a, b) ((a) & (C64(1) << (b)))

/* move definitions */
typedef u16 move_t;
typedef u32 move_ext_t;

/* the move kind is stored in the upper 4 bits of the u16 */
enum move_kind { QUIET = 0, DOUBLE_PAWN, KING_CASTLE, QUEEN_CASTLE,
				 CAPTURE, EP_CAPTURE, KNIGHT_PROMO = 8, BISHOP_PROMO,
				 ROOK_PROMO, QUEEN_PROMO, KNIGHT_PROMO_CAP,
				 BISHOP_PROMO_CAP, ROOK_PROMO_CAP, QUEEN_PROMO_CAP } move_kind_t;

/* from, to, kind */
#define move_from(m) ((m) & 0x003f)
#define move_to(m)   (((m) & 0x0fc0) >> 6)
#define move_kind(m) (((m) & 0xf000) >> 12)

#define is_promotion(m) (move_kind(m) & 0x08)
#define to_move(f, t, k) (((f) & 0x3f) | (((t) & 0x3f) << 6) | (((k) & 0x0f) << 12))

/* these macros return a bitboard with all bits set above and below the given bit */
#define upper_bits(x) (C64(~1) << x)
#define lower_bits(x) ((C64(1) << x) - 1)

/* these macros return the LS1B bitboard */
#define ls1b(x) 	  ((x) & -(x))
#define ls1b_above(x) ((x) ^ -(x))
#define ls1b_below(x) (~(x) & ((x) - 1))
#define ls1b_with(x)  ((x) ^ ((x) - 1))

#define MAX_MOVES 256
#define MAX_HISTORY 2048

/* move list */
typedef struct movelist {
	move_t moves[MAX_MOVES];
	u16 count;
} movelist_t;

/* undo record */
typedef struct undo_record {
	move_t move;		/* the move made */
	u8 castle;			/* castling permissions prior to move */
	u8 enpas;			/* en passant square prior to move */
	u32 plies;			/* fifty move count prior to move */
	piece_t capture;	/* piece captured by move */
	u64 key;			/* hash key of board prior to move */
} undo_record_t;

/* undo list */
typedef struct undolist {
	undo_record_t undo[MAX_HISTORY];
	u32 count;
} undolist_t;

/* board structure */
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

	/* half move count of current search */
	u32 search_ply;

	/* the en passant square */
	u8 enpas;

	/* side color */
	side_color_t side;

	/* side castling permissions as a bit mask */
	u8 castle;

	/* zobrist hash key */
	u64 key;
} board_t;
