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
#include <stdlib.h>
#include <string.h>

#include "ace_global.h"
#include "ace_types.h"
#include "ace_fen.h"
#include "ace_pgn.h"

static void init_knights()
{
	const int knight_moves[8] = { 10, -6, -15, -17, -10, 6, 15, 17 };
	int i, j;
	u8 index;
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
	int i, j, k;
	u8 index;
	int r, f;
#ifdef CALCULATE_RAYS
	u64 *ray = NULL;
#endif

	/* initialize the bishop and diagonal queen movelists */
	for (i = 0; i < 64; i++) {
		/* bishop slides: +9, +7, -9, -7 */
		for (j = 0; j < 4; j++) {
			index = i;
			r = rank(index);
			f = file(index);

#ifdef CALCULATE_RAYS
			switch (j) {
				case 0: ray = &ray_top[i]; break;
				case 1: ray = &ray_right[i]; break;
				case 2: ray = &ray_bottom[i]; break;
				case 3: ray = &ray_left[i]; break;
			}

			assert(ray);
#endif

			/* slide up to 7 squares in each direction */
			for (k = 0; k < 7; k++) {
				index += bishop_slides[j];

				/* add bishop & queen moves */
				if (is_valid_index(index) &&
					(abs(r - rank(index)) <= 1) &&
					(abs(f - file(index)) <= 1)) {
#ifdef CALCULATE_RAYS
					(*ray) |= (1ULL << index);
#endif
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
	int i, j, k;
	u8 index;
	int r, f;
#ifdef CALCULATE_RAYS
	u64 *ray = NULL;
#endif

	/* initialize the rook and horiz/vert queen movelists */
	for (i = 0; i < 64; i++) {
		/* rook slides: +1, +8, -8, -1 */
		for (j = 0; j < 4; j++) {
			index = i;
			r = rank(index);
			f = file(index);

#ifdef CALCULATE_RAYS
			switch (j) {
				case 0: ray = &ray_top[i]; break;
				case 1: ray = &ray_right[i]; break;
				case 2: ray = &ray_bottom[i]; break;
				case 3: ray = &ray_left[i]; break;
			}

			assert(ray);
#endif

			/* slide up to 7 squares in each direction */
			for (k = 0; k < 7; k++) {
				index += rook_slides[j];

				/* add rook & queen moves */
				if (is_valid_index(index) &&
					(abs(r - rank(index)) <= 1) &&
					(abs(f - file(index)) <= 1)) {
#ifdef CALCULATE_RAYS
					(*ray) |= (1ULL << index);
#endif
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
	int i, j;
	u8 index;
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


static void init_castling()
{
	/* [WHITE/BLACK][KINGSIDE/QUEENSIDE] */
	castle_movelist[WK] = (1ULL << G1);
	castle_movelist[WQ] = (1ULL << C1);
	castle_movelist[BK] = (1ULL << G8);
	castle_movelist[BQ] = (1ULL << C8);

	castle_unocc[WK] = 0x0000000000000060ULL;
	castle_unocc[WQ] = 0x000000000000000eULL;
	castle_unocc[BK] = 0x6000000000000000ULL;
	castle_unocc[BQ] = 0x0e00000000000000ULL;

	castle_ray[WK] = 0x0000000000000060ULL;
	castle_ray[WQ] = 0x000000000000000cULL;
	castle_ray[BK] = 0x6000000000000000ULL;
	castle_ray[BQ] = 0x0c00000000000000ULL;

	castle_side[WK] = KING_CASTLE;
	castle_side[WQ] = QUEEN_CASTLE;
	castle_side[BK] = KING_CASTLE;
	castle_side[BQ] = QUEEN_CASTLE;

	castle_rook_from[C1] = A1;
	castle_rook_to[C1] = D1;
	castle_rook_from[C8] = A8;
	castle_rook_to[C8] = D8;
	castle_rook_from[G1] = H1;
	castle_rook_to[G1] = F1;
	castle_rook_from[G8] = H8;
	castle_rook_to[G8] = F8;
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
	int i, c, j;
	u8 index;
	int r, f;

	for (i = 0; i < 64; i++) {
		r = rank(i);
		f = file(i);

		for (c = 0; c < 2; c++) {
			/* moves */
			index = i + pawn_moves[c];

			if ((c == WHITE) && (rank(i) == R1)) continue;
			if ((c == BLACK) && (rank(i) == R8)) continue;

			if (is_valid_index(index)) {
				pawn_movelist[c][i] |= (1ULL << index);
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

	pawn_enpas[0] = 0x0000000000ff0000ULL;
	pawn_enpas[1] = 0x0000ff0000000000ULL;
	pawn_promotion[0] = 0xff00000000000000ULL;
	pawn_promotion[1] = 0x00000000000000ffULL;

	for (i = 0; i < 8; i++) {
		pawn_enpas_move[WHITE][i] = (1ULL << from_rank_file(R4, i));
		pawn_enpas_move[BLACK][i] = (1ULL << from_rank_file(R5, i));
	}
}


static void init_evalboards()
{
	u32 i, c, r, f;
	u32 s, e;

	/* fill in the files */
	for (c = 0; c < 2; c++) {
		for (i = 0; i < 64; i++) {
			s = (file(i) - 1 < 0 ? 0 : file(i) - 1);
			e = (file(i) + 1 > FH ? FH : file(i) + 1);
			pawn_passed[c][i] = 0ULL;

			for (f = s; f <= e; f++) {
				pawn_passed[c][i] |= bboard_files[f];
			}
		}	
	}

	/* remove the ranks below/above the pawn */
	for (i = 0; i < 64; i++) {
		for (c = 0; c < 2; c++) {
			if (c == WHITE) {
				s = R1;
				e = rank(i);
			} else {
				s = rank(i);
				e = R8;
			}

			for (r = s; r <= e; r++) {
				pawn_passed[c][i] &= ~bboard_ranks[r];
			}
		}	
	}

	/* create isolated pawn maps */
	pawn_isolated[0] = bboard_files[FB];
	for (i = 1; i < 7; i++) {
		pawn_isolated[i] = (bboard_files[i - 1] | bboard_files[i + 1]);
	}
	pawn_isolated[7] = bboard_files[FG];
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

		castle_rook_to[i] = INVALID_SQUARE;
		castle_rook_from[i] = INVALID_SQUARE;
	}

	pawn_enpas[0] = 0ULL;
	pawn_enpas[1] = 0ULL;
	pawn_promotion[0] = 0ULL;
	pawn_promotion[1] = 0ULL;

	for (i = 0; i < 8; i++) {
		pawn_enpas_move[0][i] = 0ULL;
		pawn_enpas_move[1][i] = 0ULL;
	}

	for (i = 0; i < 16; i++) {
		castle_movelist[i] = 0ULL;
		castle_unocc[i] = 0ULL;
		castle_ray[i] = 0ULL;
		castle_side[i] = 0;
	}

	init_knights();
	init_kings();
	init_horizvert();
	init_diagonals();
	init_pawns();
	init_castling();

	init_evalboards();
}


void init_app(app_t *app)
{
	assert(app);

	app->mode = IACE;
	app->quit = FALSE;
	app->game.board = (board_t *)malloc(sizeof(board_t));

	if (app->game.board) {
		memset(app->game.board, 0, sizeof(board_t));
		memset(app->game.board->pos.squares, INVALID_PIECE, 64 * sizeof(u8));

		init_startpos(app);
	}

	/* clear search info */
	memset(&app->search, 0, sizeof(searchdata_t));

	/* set search defaults */
	app->search.flags |= SEARCH_STOPPED;
	app->search.depth = 8;

	/* allocate tree */
	app->tree.root = pgnnode_alloc();

	app->tree.count = 0;
	app->tree.depth = 0;

	/* 16 mb hash to start */
	init_hash(&app->hash, 256);
}


void init_startpos(app_t *app)
{
	fen_state_t fen;
	assert(app);

	fen_init(&fen);
	fen_use_ptr(&fen, app->game.board);
	fen_parse(&fen, FEN_OPENING, strlen(FEN_OPENING));
	fen_destroy(&fen);
	app->game.undo.count = 0;
}


void destroy_app(app_t *app)
{
	assert(app);

	free(app->tree.root);
	app->tree.count = 0;
	app->tree.depth = 0;

	free(app->game.board);
	xfree(app->hash.record);
}


void new_game(game_t* game)
{
	fen_state_t fen;

	/* initialize our ace board, we will use this to validate moves */
	memset(game->board, 0, sizeof(board_t));
	memset(game->board->pos.squares, INVALID_PIECE, 64 * sizeof(u8));

	game->undo.count = 0;

	/* initialize the starting position from FEN */
	fen_init(&fen);
	fen_use_ptr(&fen, game->board);
	fen_parse(&fen, FEN_OPENING, strlen(FEN_OPENING));
	fen_destroy(&fen);
}
