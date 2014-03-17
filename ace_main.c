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

#include "ace_types.h"
#include "ace_global.h"
#include "ace_display.h"
#include "ace_fen.h"
#include "ace_zobrist.h"
#include "ace_magic.h"

#ifndef _DEBUG
int main(int argc, char **argv)
#else
int main2(int argc, char **argv)
#endif
{
#if 0
	fen_state_t fen;
	ms_time_t tm_before, tm_after;
	char sz[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
	int depth;
	u64 nodes;
	undolist_t ul;

	assert(FALSE);

	fen_init(&fen);
	fen_parse(&fen, sz, strlen(sz));

	print_board(fen.board);

	ul.count = 0;
	depth = 5;

	get_current_tick(&tm_before);
	nodes = perft(fen.board, &ul, depth);
	get_current_tick(&tm_after);

	printf("nodes visited at D%d: %lld [%lld ms]\n", depth, nodes, get_interval(&tm_before, &tm_after));

	fen_destroy(&fen);
#endif
	init_zobrist(0xd4e12c77);
	init_magic();
	init_movelists();

	perft_kiwipete();
	printf("-----\n");
	pertf_runtests();

	return 0;
}
