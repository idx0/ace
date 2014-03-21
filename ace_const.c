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

#include "ace_global.h"
#include "ace_types.h"


u32 bitscan_8bit[256] = {
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3,
	0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0,
	1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,
	0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0,
	2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2,
	0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0,
	1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1,
	0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0,
	3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0,
	1, 0, 2, 0, 1, 0
};

u8 castle_permission[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	 7, 15, 15, 15,  3, 15, 15, 11
};
/* Piece-Square Tables */
int pawn_pcsq[64] = {
	 0,   0,   0,   0,   0,   0,   0,   0,
	-6,  -4,   1,   1,   1,   1,  -4,  -6,
	-6,  -4,   1,   2,   2,   1,  -4,  -6,
	-6,  -4,   2,   8,   8,   2,  -4,  -6,
	-6,  -4,   5,  10,  10,   5,  -4,  -6,
	-4,  -4,   1,   5,   5,   1,  -4,  -4,
	-6,  -4,   1, -24, -24,   1,  -4,  -6,
	 0,   0,   0,   0,   0,   0,   0,   0
};
int knight_pcsq[64] = {
	-8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
	-8,   0,   0,   0,   0,   0,   0,  -8,
	-8,   0,   4,   4,   4,   4,   0,  -8,
	-8,   0,   4,   8,   8,   4,   0,  -8,
	-8,   0,   4,   8,   8,   4,   0,  -8,
	-8,   0,   4,   4,   4,   4,   0,  -8,
	-8,   0,   1,   2,   2,   1,   0,  -8,
	-8,  -12, -8,  -8,  -8,  -8, -12,  -8
};
int bishop_pcsq[64] = {
	-4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
	-4,   0,   0,   0,   0,   0,   0,  -4,
	-4,   0,   2,   4,   4,   2,   0,  -4,
	-4,   0,   4,   6,   6,   4,   0,  -4,
	-4,   0,   4,   6,   6,   4,   0,  -4,
	-4,   1,   2,   4,   4,   2,   1,  -4,
	-4,   2,   1,   1,   1,   1,   2,  -4,
	-4,  -4, -12,  -4,  -4, -12,  -4,  -4
};
int rook_pcsq[64] = {
	 5,   5,   5,   5,   5,   5,   5,   5,
	20,  20,  20,  20,  20,  20,  20,  20,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	 0,   0,   0,   2,   2,   0,   0,   0
};
int queen_pcsq[64] = {
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   1,   1,   1,   1,   0,   0,
	 0,   0,   1,   2,   2,   1,   0,   0,
	 0,   0,   2,   3,   3,   2,   0,   0,
	 0,   0,   2,   3,   3,   2,   0,   0,
	 0,   0,   1,   2,   2,   1,   0,   0,
	 0,   0,   1,   1,   1,   1,   0,   0,
	-5,  -5,  -5,  -5,  -5,  -5,  -5,  -5
};
int king_pcsq[64] = {
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -40, -40, -40, -40, -40, -40, -40, -40,
    -15, -15, -20, -20, -20, -20, -15, -15,
      0,  20,  30, -30,   0, -20,  30,  20
};
int king_endgame_pcsq[64] = {
	 0,  10,  20,  30,  30,  20,  10,   0,
	10,  20,  30,  40,  40,  30,  20,  10,
	20,  30,  40,  50,  50,  40,  30,  20,
	30,  40,  50,  60,  60,  50,  40,  30,
	30,  40,  50,  60,  60,  50,  40,  30,
	20,  30,  40,  50,  50,  40,  30,  20,
	10,  20,  30,  40,  40,  30,  20,  10,
	 0,  10,  20,  30,  30,  20,  10,   0
};

int move_score_mvvlva[6][6] = {
	{ 10105, 10205, 10305, 10305, 10505, 10605 },
	{ 10104, 10204, 10304, 10304, 10504, 10604 },
	{ 10103, 10203, 10303, 10303, 10503, 10603 },
	{ 10102, 10202, 10302, 10302, 10502, 10602 },
	{ 10101, 10201, 10301, 10301, 10501, 10601 },
	{ 10100, 10200, 10300, 10300, 10500, 10600 }
};

u64 bboard_files[8] = {
	0x8080808080808080ULL,
	0x4040404040404040ULL,
	0x2020202020202020ULL,
	0x1010101010101010ULL,
	0x0808080808080808ULL,
	0x0404040404040404ULL,
	0x0202020202020202ULL,
	0x0101010101010101ULL
};

u64 bboard_ranks[8] = {
	0x00000000000000ffULL,
	0x000000000000ff00ULL,
	0x0000000000ff0000ULL,
	0x00000000ff000000ULL,
	0x000000ff00000000ULL,
	0x0000ff0000000000ULL,
	0x00ff000000000000ULL,
	0xff00000000000000ULL
};

/* move scores for certain special moves (en passant captures, castles, and
   promotions are non-zero */
int move_score_special[16] = {
	0, 0, 10050, 10050, 0, 10105, 0, 0,
	/* I am explicitly not weights a pawn capture + promotion higher than a
	   normal pawn capture - this has not been verified experimentally */
	10010, 10020, 10030, 10040, 10205, 10305, 10405, 10505
};

u64 board_colors = 0x55aa55aa55aa55aaULL;

int pawn_score_isolated = -10;
int pawn_score_passed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };
int pawn_score_backward[8] = { 0, -5, -10, -20, -35, -60, 0, 0 };

/* TODO */
u64 pawn_passed[2][64];
u64 pawn_isolated[8];

board_rank_t pawn_enpas_rank[2] = { R3, R6 };
board_rank_t pawn_double_rank[2] = { R4, R5 };
piece_type_t promoted_type[4] = { KNIGHT, BISHOP, ROOK, QUEEN };
int piece_material_values[6] = { 100, 320, 325, 521, 954, 420 };

u32 flipsq[2][64] = {
	{ A1, B1, C1, D1, E1, F1, G1, H1,
	  A2, B2, C2, D2, E2, F2, G2, H2,
	  A3, B3, C3, D3, E3, F3, G3, H3,
	  A4, B4, C4, D4, E4, F4, G4, H4,
	  A5, B5, C5, D5, E5, F5, G5, H5,
	  A6, B6, C6, D6, E6, F6, G6, H6,
	  A7, B7, C7, D7, E7, F7, G7, H7,
	  A8, B8, C8, D8, E8, F8, G8, H8 },
	{ H8, G8, F8, E8, D8, C8, B8, A8,
	  H7, G7, F7, E7, D7, C7, B7, A7,
	  H6, G6, F6, E6, D6, C6, B6, A6,
	  H5, G5, F5, E5, D5, C5, B5, A5,
	  H4, G4, F4, E4, D4, C4, B4, A4,
	  H3, G3, F3, E3, D3, C3, B3, A3,
	  H2, G2, F2, E2, D2, C2, B2, A2,
	  H1, G1, F1, E1, D1, C1, B1, A1 }
};
