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

#ifdef _DEBUG
int main2(int argc, const char **argv)
{
	fen_state_t fen;
	ms_time_t tm_before, tm_after;

	move_t m;
	int i = 0, c;
	char buf[256];
	undolist_t ul;
	int q = FALSE;
	int nmoves;

	init_zobrist(0xd4e12c77);
	init_magic();
	init_movelists();

	fen_init(&fen);

	fen_parse(&fen, FEN_OPENING, strlen(FEN_OPENING));

	ul.count = 0;

	while (!q) {
		i = 0;
		print_board(fen.board);
		printf("\nmove> ");
		fflush(stdout);
		while (i < 256) {
			c = getchar();

			if ((c == EOF) || (c == '\r') || (c == '\n')) {
				buf[i] == 0;
				break;
			}

			buf[i] = c;
			i++;
		}

		buf[i] = 0;

		if (!(q = command_quit(buf, i))) {
			nmoves = process_moves(fen.board, &ul, buf, i, fen.board->side);

			printf("-- processed %d moves --\n", nmoves);
		}
	}

	fen_destroy(&fen);

	return 0;
}
#endif
