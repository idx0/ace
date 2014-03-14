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
	fen_state_t fen;
	char sz[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	char kiwipete[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

	init_zobrist(0xd4e12c77);
	init_magic();
	init_movelists();

/*
	print_bboard(inner_squares);
	printf("\n");
*/

	fen_init(&fen);

	fen_parse(&fen, kiwipete, strlen(kiwipete));

	print_board(fen.board);

	/* magic board move generation example: */
	/* get the "first" white queen */
	u32 qsq = ACE_LSB64(fen.board->piece[WHITE][QUEEN]);
	/* get full board occupancy bitboard */
	u64 occ = (fen.board->occ[WHITE] | fen.board->occ[BLACK]);
	/* use magic bitboards to get the squares this queen can move to */
	u64 moves = magic_queen(qsq, occ);
	print_bboard(moves);
	/* print quiet moves */
	print_bboard(moves & ~occ);
	/* print moves (including captures) */
	print_bboard(moves & ~fen.board->occ[WHITE]);
	/* print capture moves */
	print_bboard(moves & fen.board->occ[BLACK]);

/*
	printf("\n%d\n", ACE_MSB64(0x0800000000000010));
*/

	fen_destroy(&fen);

	return 0;
}
#endif
