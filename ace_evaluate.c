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

#include <assert.h>

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_global.h"


static int check_material_draw(board_t *board)
{
	assert(board);

	/* king vs. king */
	if (board->pos.npieces == 2) return TRUE;

	/* king & bishop vs. king */
	if ((board->pos.npieces == 3) &&
		(((board->pos.minors[WHITE] == 1) && (!board->pos.piece[WHITE][KNIGHT])) ||
		 ((board->pos.minors[BLACK] == 1) && (!board->pos.piece[BLACK][KNIGHT]))))
	{
		return TRUE;
	}

	/* king & knight vs. king */
	if ((board->pos.npieces == 3) &&
		(((board->pos.minors[WHITE] == 1) && (!board->pos.piece[WHITE][BISHOP])) ||
		 ((board->pos.minors[BLACK] == 1) && (!board->pos.piece[BLACK][BISHOP]))))
	{
		return TRUE;
	}

	/* king & bishop vs. king & bishop, bishops on the same color */
	if ((board->pos.npieces == 4) &&
		(board->pos.minors[WHITE] == 1) && (!board->pos.piece[WHITE][KNIGHT]) &&
		(board->pos.minors[BLACK] == 1) && (!board->pos.piece[BLACK][KNIGHT]))
	{
		/* we have a king and 1 bishop per side */
		if (((board->pos.piece[WHITE][BISHOP] & board_colors) &&
			 (board->pos.piece[BLACK][BISHOP] & board_colors)) ||
			(!(board->pos.piece[WHITE][BISHOP] & board_colors) &&
			 !(board->pos.piece[BLACK][BISHOP] & board_colors)))
		{
			return TRUE;
		}
	}

	return FALSE;
}


static int evaluate_pawns(board_t *board, const side_color_t c)
{
	int score = 0;
	u64 bb;
	u32 sq;
	side_color_t oc = (~c & 0x01);

	assert(board);

	bb = board->pos.piece[c][PAWN];

	while (bb) {
		sq = ACE_LSB64(bb);

		score += pawn_pcsq[sq];

		if (pawn_isolated[sq] & board->pos.piece[c][PAWN]) {
			score += pawn_score_isolated;
		}

		if (pawn_passed[c][sq] & board->pos.piece[oc][PAWN]) {
			score += pawn_score_passed[rank(sq)];
		}

		if (pawn_passed[c][sq] & board->pos.piece[c][PAWN]) {
			score += pawn_score_backward[rank(sq)];
		}

		bb ^= (1ULL << sq);
	}

	return score;
}


int evaluate(board_t* board)
{
	const int sign[2] = { 1, -1 };
	int score = 0, c;

	assert(board);

	/* first, just subtract the materials */
	score = board->pos.material[WHITE] - board->pos.material[BLACK];

	/* check for draws */
	if (check_material_draw(board)) return 0;

	/* evaluate pieces */
	for (c = 0; c < 2; c++) {
		score += sign[c] * evaluate_pawns(board, c);
	}


	/* flip the value of score based on side (or else this function will be
	   asymetrical) */
	if (board->side == WHITE) {
		return score;
	} else {
		return -score;
	}
}
