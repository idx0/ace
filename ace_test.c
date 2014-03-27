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

#include <stdio.h>
#include <assert.h>

#include <string.h>

#include "ace_types.h"
#include "ace_global.h"
#include "ace_display.h"
#include "ace_fen.h"
#include "ace_zobrist.h"
#include "ace_magic.h"

void test()
{
	u64 pieces, occ, att;
	u64 blks[2], xray[2][2];
	fen_state_t fen;
	board_t board;
	u32 sq;
	char sz[] = "r3r1k1/pppq2pp/2n1bb2/2B1pp2/8/3P1NP1/PPQ1PPBP/R2R2K1 w - - 0 1";

	fen_init(&fen);
	fen_use_ptr(&fen, &board);
	fen_parse(&fen, sz, strlen(sz));
	fen_destroy(&fen);

	occ = (board.pos.occ[WHITE] | board.pos.occ[BLACK]);
	pieces = board.pos.piece[WHITE][QUEEN];

	print_board(&board);

//	while (pieces) {
		sq = ACE_MSB64(pieces);

		att = magic_queen(sq, occ);

		/* get blockers */
		blks[WHITE] = att & board.pos.occ[WHITE];
		blks[BLACK] = att & board.pos.occ[BLACK];

		/* subtract blockers */
		xray[WHITE][WHITE] = magic_queen(sq, occ ^ blks[WHITE]);
		xray[BLACK][WHITE] = magic_queen(sq, occ ^ blks[BLACK]);
		xray[WHITE][BLACK] = xray[WHITE][WHITE];
		xray[BLACK][BLACK] = xray[BLACK][WHITE];

		/* print blocked movement */
		print_bboard2(xray[WHITE], xray[BLACK]);

		xray[WHITE][WHITE] &= ~blks[WHITE] & board.pos.occ[WHITE];
		xray[WHITE][BLACK] &= ~blks[BLACK] & board.pos.occ[BLACK];
		xray[BLACK][WHITE] &= ~blks[WHITE] & board.pos.occ[WHITE];
		xray[BLACK][BLACK] &= ~blks[BLACK] & board.pos.occ[BLACK];

		/* print xrays */
		printf("x-rays\n       w -> w              w -> b\n");
		print_bboard2(xray[WHITE][WHITE], xray[WHITE][BLACK]);
		printf("x-rays\n       b -> w              b -> b\n");
		print_bboard2(xray[BLACK][WHITE], xray[BLACK][BLACK]);

//		pieces ^= (1ULL << sq);
//	}
}
