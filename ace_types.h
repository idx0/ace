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
typedef enum piece_type { PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING } piece_type_t;

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
/*  0 0 0 0 quiet
    0 0 0 1 double pawn
    0 0 1 0 king castle
    0 0 1 1 queen castle
    0 1 0 0 capture
    0 1 0 1 ep capture
    1 0 0 0 knight promotion
    1 0 0 1 bishop promotion
    1 0 1 0 rook promotion
    1 0 1 1 queen promotion
    1 1 0 0 knight promotion via capture
    1 1 0 1 bishop promotion via capture
    1 1 1 0 rook promotion via capture
    1 1 1 1 queen promotion via capture
*/
enum move_kind { QUIET = 0, DOUBLE_PAWN, KING_CASTLE, QUEEN_CASTLE,
				 CAPTURE, EP_CAPTURE, KNIGHT_PROMO = 8, BISHOP_PROMO,
				 ROOK_PROMO, QUEEN_PROMO, KNIGHT_PROMO_CAP,
				 BISHOP_PROMO_CAP, ROOK_PROMO_CAP, QUEEN_PROMO_CAP } move_kind_t;

/* from, to, kind */
#define move_from(m) ((m) & 0x003f)
#define move_to(m)   (((m) & 0x0fc0) >> 6)
#define move_kind(m) (((m) & 0xf000) >> 12)

#define is_promotion(m) (move_kind(m) & 0x08)
#define is_capture(m) (move_kind(m) & 0x04)
#define to_move(f, t, k) (((f) & 0x3f) | (((t) & 0x3f) << 6) | (((k) & 0x0f) << 12))

/* these macros return a bitboard with all bits set above and below the given bit */
#define upper_bits(x) (C64(~1) << x)
#define lower_bits(x) ((C64(1) << x) - 1)

/* these macros return the LS1B bitboard */
#define ls1b(x) 	  ((x) & -(x))
#define ls1b_above(x) ((x) ^ -(x))
#define ls1b_below(x) (~(x) & ((x) - 1))
#define ls1b_with(x)  ((x) ^ ((x) - 1))

#define get_sign(x, s) (((x) & (1 << (s - 1))) < 0 ? -1 : 1)

#define MAX_MOVES 256
#define MAX_HISTORY 2048

/* move list */
typedef struct movelist {
	move_t moves[MAX_MOVES];
	int scores[MAX_MOVES];
	u16 count;
} movelist_t;

/* undo record */
typedef struct undo_record {
	move_t move;		/* the move made */
	u8 castle;			/* castling permissions prior to move */
	u8 enpas;			/* en passant square prior to move */
	u32 ply;			/* fifty move count prior to move */
	piece_t capture;	/* piece captured by move */
	u64 key;			/* hash key of board prior to move */
} undo_record_t;

/* undo list */
typedef struct undolist {
	undo_record_t undo[MAX_HISTORY];
	u32 count;
} undolist_t;

typedef struct eval_cache {
	/* A bitboard representing the sqaures defended by a color */
	u64 defend[2];
	/* A bitboard representing the squares attacked by a color */
	u64 attack[2];
	/* A table representing the mobility of the piece at a given square */
	u8 mobility[64];
	/* This variable is set to true if the cache data is valid */
	int valid;
} eval_cache_t;

typedef struct position {
	/* bitboard which represents all pieces */
	u64 piece[2][6];
	/* an array of each piece on each square */
	piece_t squares[64];
	/* major pieces: rooks & queens */
	u8 majors[2];
	/* minor pieces: bishops & knights */
	u8 minors[2];
	/* a bitboard of all pawns */
	u64 pawns;
	/* the king square */
	u32 king_sq[2];
	/* total number of pieces on the board */
	u8 npieces;
	/* material count */
	int material[2];
	/* bitboard which represents all occupied sq for this color */
	u64 occ[2];
	/* a cache of values pre-computed for free during move generation that can
	   be later used to help evaluation */
	eval_cache_t cache;
} position_t;

#define HASH_NONE	0
#define HASH_ALPHA	1
#define HASH_BETA	2
#define HASH_EXACT	4
#define HASH_SEED	8

/**
 * 128 bit hash entry
 *   8 byte key
 *   2 byte move
 *   2 byte score
 *   1 byte age
 *   2 byte depth (max is 64 so this should be fine)
 *   1 byte flag
 */
typedef struct hash_record {
	u64 key;		/* the position hash key for this entry */
	u64 move  : 16;	/* the move that was made to achieve this entry */
	u64 score : 16;	/* the evaluation score for this entry */
	u64 age   : 8;	/* the age of this hash record (in searches) */
	u64 depth : 16;	/* the depth at which this entry was found */
	u64 flags : 8;	/* the flags for this entry */
} hash_record_t;

typedef struct hash_cluster {
	hash_record_t h[4];
} hash_cluster_t;

typedef struct hash_table {
	/* A pointer to the aligned memory hash table */
	hash_cluster_t *record;
	/* The number of entries in the table */
	int entries;
	/* The number of entries overwritten during search */
	int overwritten;
	/* The number of hash hits */
	u32 hit;
	/* The number of hash hits that cause a branch cutoff */
	u32 cut;
	/* The size of the has table as a power of 2 */
	u32 size;
	/* The total number of hash record entries existing in the table */
	u32 exist;
	/* the number of generations of this table */
	u8 generations;
} hash_table_t;

#define SEARCH_MAXDEPTH	64

#define NODE_ROOT		1 	/* flag indicating the node is the root node */
#define NODE_CHECK		2 	/* flag indicating the side is in check on this node */
#define NODE_NULL		4	/* flag indicating null moves are allowed on
							   children of this node */

/* This structure holds node relavent information.  It is contained in the
   stack of the alpha-beta function and passed to each child */
typedef struct node {
	/* the best move found so far by this node */
	move_t best;
	/* flags set for this node */
	u16 flags;
	/* the move list for this node */
	movelist_t ml;
	/* the capture list for this node */
	movelist_t cl;
	/* number of moves actually made by this node */
	u16 made;
} node_t;

typedef const node_t* const cnodeptr_t;

/* board structure */
typedef struct board {
	/* borad position definitions */
	position_t pos;
	/* half move count of current search */
	/* note: this is, in most cases, equal to the depth of the search.  For
	   searches using a single undo list, this should alway be equal to
	   undolist_t::count */
	u32 ply;
	/* game half moves */
	u32 half;
	/* game full moves */
	u32 moves;
	/* the en passant square */
	u8 enpas;
	/* side color */
	side_color_t side;
	/* side castling permissions as a bit mask */
	u8 castle;
	/* zobrist hash key */
	u64 key;
	/* move ordering variables */
	int killers[2][SEARCH_MAXDEPTH];
	int history[2][6][64];
} board_t;

typedef enum input_mode { IACE, IUCI, IXBOARD, IMOVE, IFEN } input_mode_t;

#define SEARCH_DEPTH	1
#define SEARCH_TIMED	2
#define SEARCH_STOPPED	4
#define SEARCH_PONDER	8
#define SEARCH_INFINITE	16
#define SEARCH_NULL		32

typedef struct searchdata {
	/* start and end times */
	ms_time_t start;
	ms_time_t end;
	/* depth */
	int depth;
	/* moves remaining */
	int movesleft;
	/* number of nodes visited */
	u64 nodes;
	/* search flags for this search */
	u32 flags;
	/* pv line for this search */
	move_t pv[SEARCH_MAXDEPTH];
} searchdata_t;

typedef struct ace_app {
	/* the board, allocated by the application */
	board_t* board;
	/* the global undo list for all moves made by the application */
	undolist_t ul;
	/* true if the application should quit */
	int quit;
	/* the current input mode */
	input_mode_t mode;
	/* search data for this app */
	searchdata_t search;
	/* application wide transposition table */
	hash_table_t hash;
	/* the side the application is playing */
	side_color_t side;
} app_t;
