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
int main(int argc, const char **argv)
{
#if 0
	fen_state_t fen;
	ms_time_t tm_before, tm_after;
	char eptest[] = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1";
	char sz[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	char kiwipete[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	char promo_debug[] = "1r5k/2P5/8/8/8/8/P5p1/K7 w - - 0 1";

	int depth;
	u64 nodes;
	undolist_t ul;

	init_zobrist(0xd4e12c77);
	init_magic();
	init_movelists();

	fen_init(&fen);

	fen_parse(&fen, eptest, strlen(eptest));

	print_board(fen.board);

	ul.count = 0;
	depth = 3;

	get_current_tick(&tm_before);
	nodes = perft(fen.board, &ul, depth);
	get_current_tick(&tm_after);

	printf("nodes visited at D%d: %lld [%lld ms]\n", depth, nodes, get_interval(&tm_before, &tm_after));

	fen_destroy(&fen);
#endif

	init_zobrist(0xd4e12c77);
	init_magic();
	init_movelists();

	pertf_runtests();

	return 0;
}
#endif
