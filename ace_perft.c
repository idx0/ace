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
#include <string.h>

#include "ace_global.h"
#include "ace_types.h"
#include "ace_fen.h"

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include "ace_display.h"
#endif

#define PERFT_TESTFILE "perftsuite.epd"

void perft_kiwipete()
{
	u32 i;
	u64 nodes;
	fen_state_t fen;
	undolist_t ul;
	ms_time_t tm_before, tm_after;

	u64 d[5] = { 48, 2039, 97862, 4085603, 193690690 };

	fen_init(&fen);
	fen_parse(&fen, FEN_KIWIPETE, strlen(FEN_KIWIPETE));

	for (i = 0; i < 5; i++) {
		ul.count = 0;

		get_current_tick(&tm_before);
		nodes = perft(fen.board, &ul, i + 1);
		get_current_tick(&tm_after);

		printf("perft %s at depth %d: %lld [%lld ms]\n",
			(nodes == d[i] ? "passed" : "failed"), i + 1, nodes,
			get_interval(&tm_before, &tm_after));
	}

	fen_destroy(&fen);
}


void pertf_runtests()
{
	static char buf[1024];
	u64 d[6], nodes;
	u32 passed, cnt = 0, pcnt = 0;
	FILE *fp;
	int i;
	fen_state_t fen;
	undolist_t ul;
	ms_time_t tm_before, tm_after;

#ifdef PRINT_DEBUG
	printf("WARNING: DEBUG PRINTING IS ON!\n");
#endif

	fp = fopen(PERFT_TESTFILE, "r");

	fen_init(&fen);

	while (fgets(buf, 1024, fp) != NULL) {
		i = 0; while (buf[i] != ';') i++;

		fen_parse(&fen, buf, i - 1);

		sscanf(buf + i,
			";D1 %llu ;D2 %llu ;D3 %llu ;D4 %llu ;D5 %llu ;D6 %llu",
			&d[0], &d[1], &d[2], &d[3], &d[4], &d[5]);
		passed = 0;

		cnt++;
		printf("Running test %d...\n", cnt);

		for (i = 0; i < 6; i++) {
			ul.count = 0;

			get_current_tick(&tm_before);
			nodes = perft(fen.board, &ul, i + 1);
			get_current_tick(&tm_after);

			printf("  perft %s at depth %d: %lld [%lld ms]\n",
				(nodes == d[i] ? "passed" : "passed"), i + 1, nodes,
				get_interval(&tm_before, &tm_after));

			if (nodes == d[i]) passed++;
		}

		if (passed == 6) {
			pcnt++;
			printf("PASSED %d/6\n\n", passed);
		} else {
			printf("FAILED %d/6\n\n", passed);
		}

		memset(buf, 0, 1024);
	}

	printf("Finished all tests!\n");
	printf("  Tests run:    %d\n", cnt);
	printf("  Tests passed: %d\n", pcnt);
	if (pcnt == cnt) {
		printf("All tests PASSED!\n");
	}

	fen_destroy(&fen);

	fclose(fp);
}


u64 perft(board_t* board, undolist_t* ul, int depth)
{
	movelist_t ml;
	int nmoves, i;
	u64 nodes = 0;

	if (!depth) return 1;

	nmoves = generate_moves(board, &ml);

	for (i = 0; i < nmoves; i++) {
		if (do_move(board, ul, ml.moves[i])) {
#ifdef PRINT_DEBUG
			printf("depth %d >>>\n", depth);
			print_board(board);
			printf("\n");
#endif
			nodes += perft(board, ul, depth - 1); 
			undo_move(board, ul);
#ifdef PRINT_DEBUG
			print_board(board);
			getchar();
			printf("\n\n");
#endif
		}
	}

	return nodes;
}
