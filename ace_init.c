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

#include <assert.h>
#include <stdlib.h>

static void init_knights()
{
	const int knight_moves[8] = { 10, -6, -15, -17, -10, 6, 15, 17 };
	int i, j, index;
	int r, f;

	/* initialize the knight movelist */
	for (i = 0; i < 64; i++) {
		r = rank(i);
		f = file(i);

		for (j = 0; j < 8; j++) {
			index = i + knight_moves[j];

			if (is_valid_index(index) &&
				(abs(r - rank(index)) <= 2) &&
				(abs(f - file(index)) <= 2)) {
				knight_movelist[i] |= (1ULL << index);
			}
		}
	}
}


static void init_diagonals()
{
	const int bishop_slides[4] = { 7, 9, -7, -9 };
	int i, j, k, index;
	int r, f;
	u64 *ray = NULL;

	/* initialize the bishop and diagonal queen movelists */
	for (i = 0; i < 64; i++) {
		/* bishop slides: +9, +7, -9, -7 */
		for (j = 0; j < 4; j++) {
			index = i;
			r = rank(index);
			f = file(index);

			switch (j) {
				case 0: ray = &ray_topleft[i]; break;
				case 1: ray = &ray_topright[i]; break;
				case 2: ray = &ray_bottomright[i]; break;
				case 3: ray = &ray_bottomleft[i]; break;
			}

			assert(ray);

			/* slide up to 7 squares in each direction */
			for (k = 0; k < 7; k++) {
				index += bishop_slides[j];

				/* add bishop & queen moves */
				if (is_valid_index(index) &&
					(abs(r - rank(index)) <= 1) &&
					(abs(f - file(index)) <= 1)) {
					(*ray) |= (1ULL << index);
					bishop_movelist[i] |= (1ULL << index);
					queen_movelist[i] |= (1ULL << index);
				} else {
					break;
				}

				r = rank(index);
				f = file(index);
			}
		}
	}
}


static void init_horizvert()
{
	const int rook_slides[4] = { 8, 1, -8, -1 };
	int i, j, k, index;
	int r, f;
	u64 *ray = NULL;

	/* initialize the rook and horiz/vert queen movelists */
	for (i = 0; i < 64; i++) {
		/* rook slides: +1, +8, -8, -1 */
		for (j = 0; j < 4; j++) {
			index = i;
			r = rank(index);
			f = file(index);

			switch (j) {
				case 0: ray = &ray_top[i]; break;
				case 1: ray = &ray_right[i]; break;
				case 2: ray = &ray_bottom[i]; break;
				case 3: ray = &ray_left[i]; break;
			}

			assert(ray);

			/* slide up to 7 squares in each direction */
			for (k = 0; k < 7; k++) {
				index += rook_slides[j];

				/* add rook & queen moves */
				if (is_valid_index(index) &&
					(abs(r - rank(index)) <= 1) &&
					(abs(f - file(index)) <= 1)) {
					(*ray) |= (1ULL << index);
					rook_movelist[i] |= (1ULL << index);
					queen_movelist[i] |= (1ULL << index);
				} else {
					break;
				}

				r = rank(index);
				f = file(index);
			}
		}
	}
}


static void init_kings()
{
	const int king_moves[8] = { -1, 7, 8, 9, 1, -7, -8, -9 };
	int i, j, index;
	int r, f;

	/* initialize the king movelist */
	for (i = 0; i < 64; i++) {
		r = rank(i);
		f = file(i);

		for (j = 0; j < 8; j++) {
			index = i + king_moves[j];

			if (is_valid_index(index) &&
				(abs(r - rank(index)) <= 1) &&
				(abs(f - file(index)) <= 1)) {
				king_movelist[i] |= (1ULL << index);
			}
		}
	}
}


static void init_pawns()
{
	/* pawns can move 1 direction forward with the following exceptions:
	     - they are on their starting square (ranks 2 or 7), in which case
	       they can move 2 squares forward
	     - an piece of the opposite color is diagonal to them, in which case
	       they can capture that piece

	   this function will generate both pawn move and pawn capture bitboards
	 */

	const int pawn_captures[2][2] = { { 7, 9 }, { -7, -9 } };
	const int pawn_moves[2] = { 8, -8 };
	int i, c, j, index;
	int r, f;

	for (i = 0; i < 64; i++) {
		r = rank(i);
		f = file(i);

		for (c = 0; c < 2; c++) {
			/* moves */
			index = i + pawn_moves[c];

			if (is_valid_index(index)) {
				pawn_movelist[c][i] |= (1ULL << index);

				if ((c == 0) && (r == R2)) {
					pawn_movelist[c][i] |= (1ULL << (index + pawn_moves[c]));
					pawn_enpas[c] |= (1ULL << i);
				}

				if ((c == 1) && (r == R7)) {
					pawn_movelist[c][i] |= (1ULL << (index + pawn_moves[c]));
					pawn_enpas[c] |= (1ULL << i);
				}
			}

			/* captures */
			for (j = 0; j < 2; j++) {
				index = i + pawn_captures[c][j];

				if (is_valid_index(index) &&
					(abs(r - rank(index)) <= 1) &&
					(abs(f - file(index)) <= 1)) {
					pawn_capturelist[c][i] |= (1ULL << index);
				}
			}
		}
	}
}


void init_movelists()
{
	int i;

	for (i = 0; i < 64; i++) {
		knight_movelist[i] = 0ULL;
		queen_movelist[i] = 0ULL;
		bishop_movelist[i] = 0ULL;
		rook_movelist[i] = 0ULL;
		king_movelist[i] = 0ULL;

		pawn_movelist[0][i] = 0ULL;
		pawn_movelist[1][i] = 0ULL;

		pawn_capturelist[0][i] = 0ULL;
		pawn_capturelist[1][i] = 0ULL;

		ray_topleft[i] = 0ULL;
		ray_top[i] = 0ULL;
		ray_topright[i] = 0ULL;
		ray_right[i] = 0ULL;
		ray_bottomright[i] = 0ULL;
		ray_bottom[i] = 0ULL;
		ray_bottomleft[i] = 0ULL;
		ray_left[i] = 0ULL;
	}

	pawn_enpas[0] = 0ULL;
	pawn_enpas[1] = 0ULL;

	init_knights();
	init_kings();
	init_horizvert();
	init_diagonals();
	init_pawns();
}
