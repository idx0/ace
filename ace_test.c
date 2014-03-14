#include <stdio.h>
#include <assert.h>

#include <string.h>

#include "ace_types.h"
#include "ace_global.h"
#include "ace_display.h"
#include "ace_fen.h"
#include "ace_zobrist.h"

#ifdef _DEBUG
int main(int argc, const char **argv)
{
	fen_state_t fen;
	char sz[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


	init_zobrist(0xd4e12c77);
	init_movelists();

	print_bboard(ray_topleft[C4]);


	fen_init(&fen, sz, strlen(sz));

	print_board(fen.board);
	printf("%d\n", ACE_POPCNT64(0x0800000000000010));

	fen_destroy(&fen);

	return 0;
}
#endif
