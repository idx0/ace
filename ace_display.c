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

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_global.h"

void print_bboard(const u64 bb)
{
	int f, r;
	u8 i;

	for (r = 8; r > 0; r--) {
		printf(" %d ", r);

		for (f = 0; f < 8; f++) {
			i = from_rank_file(r - 1, f);
			assert(is_valid_index(i));

			if (set(bb, i)) {
				printf("x ");
			} else {
				printf(". ");
			}
		}

		printf("\n");
	}

	printf("   a b c d e f g h\n");
}


void print_bboard2(const u64 bb, const u64 bb2)
{
	int f, r;
	u8 i;

	for (r = 8; r > 0; r--) {
		printf(" %d ", r);

		for (f = 0; f < 8; f++) {
			i = from_rank_file(r - 1, f);
			assert(is_valid_index(i));

			if (set(bb, i)) {
				printf("x ");
			} else {
				printf(". ");
			}
		}

		printf("   ");

		for (f = 0; f < 8; f++) {
			i = from_rank_file(r - 1, f);
			assert(is_valid_index(i));

			if (set(bb2, i)) {
				printf("x ");
			} else {
				printf(". ");
			}
		}

		printf("\n");
	}

	printf("   a b c d e f g h    a b c d e f g h\n");
}


void print_board(const board_t* b)
{
	int f, r;
	u8 i;
	const char pieces[2][6] = {
		{ 'P', 'N', 'B', 'R', 'Q', 'K' },
		{ 'p', 'n', 'b', 'r', 'q', 'k' }
	};
	const char* castles[16] = { "-", "K",  "Q",  "KQ",  "k",  "Kk",  "KQ",  "KQk",
							    "q", "Kq", "Qq", "KQq", "kq", "Kkq", "Qkq", "KQkq"};
	const char ranks[8] = { '1', '2', '3', '4', '5', '6', '7', '8' };
	const char files[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

	assert(b);

	for (r = 8; r > 0; r--) {
		printf(" %d ", r);

		for (f = 0; f < 8; f++) {
			i = from_rank_file(r - 1, f);
			assert(is_valid_index(i));

			if (piece_valid(b->pos.squares[i])) {
				printf("%c ",
					pieces[piece_color(b->pos.squares[i])]
						  [piece_type(b->pos.squares[i])]);
			} else {
				printf(". ");
			}

			/* print extra stuff */
		}

		if (r == R7) {
			printf("  %s", (b->side ? "black" : "white"));
		} else if (r == R6) {
			if (is_valid_index(b->enpas)) {
				printf("  en: %c%c", files[file(b->enpas)], ranks[rank(b->enpas)]);
			} else {
				printf("  en: -");
			}
		} else if (r == R5) {
			printf("  %s", castles[b->castle]);
		} else if (r == R4) {
			printf("  %d / %d", b->half, b->moves);
		}

		printf("\n");
	}

	printf("   a b c d e f g h\n");
}


char* str_algebraic(const piece_t piece, const move_t move)
{
	static char psz[6];

	u32 to = move_to(move);
	u32 from = move_from(move);
	u8 kind = move_kind(move);
	const char pieces[2][6] = {
		{ 'P', 'N', 'B', 'R', 'Q', 'K' },
		{ 'P', 'N', 'B', 'R', 'Q', 'K' }
	};
	const char ranks[8] = { '1', '2', '3', '4', '5', '6', '7', '8' };
	const char files[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };

	if ((kind == CAPTURE) || (kind == EP_CAPTURE)) {
		if (piece_type(piece) == PAWN) {
			snprintf(psz, 6, "%cx%c%c", files[file(from)], files[file(to)], ranks[rank(to)]);
		} else {
			snprintf(psz, 6, "%cx%c%c", pieces[piece_color(piece)][piece_type(piece)],
				files[file(to)], ranks[rank(to)]);
		}
	} else if (is_promotion(kind)) {
		assert(piece_type(piece) == PAWN);
		snprintf(psz, 6, "%c%c=%c", files[file(to)], ranks[rank(to)],
			pieces[piece_color(piece)][promoted_type[kind & 0x03]]);
	} else if (kind == KING_CASTLE) {
		snprintf(psz, 6, "O-O");
	} else if (kind == QUEEN_CASTLE) {
		snprintf(psz, 6, "O-O-O");
	} else {
		if (piece_type(piece) == PAWN) {
			snprintf(psz, 6, "%c%c", files[file(to)], ranks[rank(to)]);
		} else {
			snprintf(psz, 6, "%c%c%c", pieces[piece_color(piece)][piece_type(piece)],
				files[file(to)], ranks[rank(to)]);
		}
	}
	/* TODO: + for check, # for checkmate */

	psz[5] = 0;

	return psz;
}


void print_algebraic(const piece_t piece, const move_t move)
{
	printf("%s", str_algebraic(piece, move));
}
