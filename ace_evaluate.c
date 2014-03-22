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

	/* this function assumes that kings will never be captured and are therefore
	   always in the piece list */

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


static int evaluate_pawns(board_t *board, const side_color_t s)
{
	int score = 0;
	u64 bb, mask, raw;
	u32 sq;
	side_color_t oc = (~s & 0x01);

	assert(board);

	raw = bb = board->pos.piece[s][PAWN];

	/* loop through all pawns on the side */
	while (bb) {
		sq = ACE_LSB64(bb);

		/* add in piece square table value */
		score += pawn_pcsq[flipsq[s][sq]];

		/* is the pawn isolated */
		if (!(pawn_isolated[file(sq)] & raw)) score += 0;
		/* is the pawn backward */
		if (ACE_POPCNT64(pawn_passed[s][sq] & raw) == 
			ACE_POPCNT64(pawn_isolated[file(sq)] & raw)) score += 0;
		/* is the pawn connected */
		mask = (pawn_isolated[file(sq)] & bboard_ranks[rank(sq)]);
		if (ACE_POPCNT64(raw & mask) > 0) score += 0;
		/* is this a passed pawn */
		if (!(pawn_passed[s][sq] & board->pos.piece[oc][PAWN])) score += 0;
		/* is this a double pawn */
		if (ACE_POPCNT64(raw & bboard_files[file(sq)]) > 1) score += 0;
		/* is a rook pawn */
		if ((file(sq) == FH) || (file(sq) == FA)) score += 0;
		/* TODO:
			check for pawns defended by other pawns
			check for pawns guarding a king on the first rank
			check for pawn chains
			checked for pinned pawns
		 */


		bb ^= (1ULL << sq);
	}

	return score;
}


int evaluate(board_t* board)
{
	int score = 0;
	side_color_t us, them;

	assert(board);

	them = (~board->side & 0x01);
	us = board->side;

	/* first, just subtract the materials */
	score = board->pos.material[us] - board->pos.material[them];

	/* check for draws */
	if (check_material_draw(board)) return 0;

	/* evaluate pieces */

	/* evaluate pawns - it may be meaningful to add a a pawn hash table which
	   can then be checked in order to forgoe pawn evaluation */
	score += evaluate_pawns(board, us);
	score -= evaluate_pawns(board, them);

	return score;
}
