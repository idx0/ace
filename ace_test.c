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
	u64 blks[2], xray[2];
	fen_state_t fen;
	board_t board;
	u32 sq;

	fen_init(&fen);
	fen_use_ptr(&fen, &board);
	fen_parse(&fen, FEN_KIWIPETE, strlen(FEN_KIWIPETE));
	fen_destroy(&fen);

	occ = (board.pos.occ[WHITE] | board.pos.occ[BLACK]);
	pieces = board.pos.piece[WHITE][BISHOP];

//	while (pieces) {
		sq = ACE_MSB64(pieces);

		att = magic_bishop(sq, occ);
		blks[WHITE] = att & board.pos.occ[WHITE];
		blks[BLACK] = att & board.pos.occ[BLACK];

		print_bboard(att);
		print_bboard(blks[WHITE]);
		print_bboard(blks[BLACK]);

//		pieces ^= (1ULL << sq);
//	}
}
