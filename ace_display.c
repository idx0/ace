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

void print_bboard(const u64 bb)
{
	int f, r, i;

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


void print_board(const board_t* b)
{
	int f, r, i;
	const char pieces[2][6] = {
		{ 'P', 'N', 'R', 'B', 'Q', 'K' },
		{ 'p', 'n', 'r', 'b', 'q', 'k' }
	};

	assert(b);

	for (r = 8; r > 0; r--) {
		printf(" %d ", r);

		for (f = 0; f < 8; f++) {
			i = from_rank_file(r - 1, f);
			assert(is_valid_index(i));

			if (piece_valid(b->squares[i].type)) {
				printf("%c ", pieces[b->squares[i].color][b->squares[i].type]);
			} else {
				printf(". ");
			}
		}

		printf("\n");
	}

	printf("   a b c d e f g h\n");
}
