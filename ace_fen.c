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
#include <string.h>

#include "ace_fen.h"

int isrank(const char c)
{
	return (isnonzero(c) && (c != '9'));
}


int isfile(const char c)
{
	return ((c >= 'a') && (c <= 'h'));
}


int isnonzero(const char c)
{
	return ((c >= '1') && (c <= '9'));
}


int iszero(const char c)
{
	return (c == '0');
}


int iswhite(const char c)
{
	return ((c == 'P') || (c == 'N') || (c == 'B') ||
			(c == 'R') || (c == 'Q') || (c == 'K'));
}


int isblack(const char c)
{
	return ((c == 'p') || (c == 'n') || (c == 'b') ||
			(c == 'r') || (c == 'q') || (c == 'k'));
}


piece_t from_char(const char c)
{
	piece_t p = INVALID_PIECE;
	set_piece_color(p, WHITE);

	/* is c a valid character */
	switch (c) {
		case 'p': set_piece_color(p, BLACK);
		case 'P': set_piece_type(p, PAWN); break;
		case 'n': set_piece_color(p, BLACK);
		case 'N': set_piece_type(p, KNIGHT); break;
		case 'b': set_piece_color(p, BLACK);
		case 'B': set_piece_type(p, BISHOP); break;
		case 'r': set_piece_color(p, BLACK);
		case 'R': set_piece_type(p, ROOK); break;
		case 'q': set_piece_color(p, BLACK);
		case 'Q': set_piece_type(p, QUEEN); break;
		case 'k': set_piece_color(p, BLACK);
		case 'K': set_piece_type(p, KING); break;
		default: break;
	}

	return p;
}


int parse_rank(fen_state_t *fen, const char *str, size_t len)
{
	static const char TERM = '/';
	int index, rc = FEN_SUCCESS;
	char c;
	piece_t p;

	u16 file_cnt, rank_cnt = 0;

	while ((rank_cnt < 8) && (rc == FEN_SUCCESS))
	{
		file_cnt = 0;
		fen->state = FEN_RANK_BEGIN;
		fen->cur_rank = rank_cnt;

		while ((fen->read_pos < len) && (fen->state == FEN_RANK_BEGIN)) {
			c = str[fen->read_pos];

			if (c == TERM) {			/* terminal symbol */
				fen->state = FEN_RANK_END;

				if (file_cnt < 8) {
					rc = ERROR_FEN_INCOMPLETE_RANK;
				}
			} else if (c == '8') {		/* full rank space */
				if (file_cnt != 0) {
					rc = ERROR_FEN_INVALID_RANK;
				}
				file_cnt = 8;
			} else if (isrank(c)) {		/* rank space */
				file_cnt += (c - '1');
			} else if (iswhite(c) || isblack(c)) {
				/* TODO: add piece */
				p = from_char(c);
				index = from_rank_file(7 - rank_cnt, file_cnt);

				if (is_valid_index(index)) {
					fen->board->piece[piece_color(p)][piece_type(p)] |= (1ULL << index);
					fen->board->occ[piece_color(p)] |= (1ULL << index);
					fen->board->squares[index] = p;
				} else {
					rc = ERROR_FEN_INVALID_MOVE;
				}

				file_cnt++;
			} else {					/* invalid char */
				fen->state = FEN_RANK_END;
				rc = ERROR_FEN_INVALID_CHAR;
			}

			/* throw an error if the file_cnt is > 8 strictly */
			if (file_cnt > 8) {
				fen->state = FEN_RANK_END;
				rc = ERROR_FEN_INVALID_RANK;
			}

			fen->read_pos++;
		}

		rank_cnt++;
	}

	return rc;
}


int fen_init(fen_state_t *fen, const char *str, size_t len)
{
	int rc = FEN_SUCCESS;

	assert(fen);

	fen->state = FEN_RANK_BEGIN;
	fen->cur_rank = R1;
	fen->read_pos = 0;

	fen->board = (board_t*)malloc(sizeof(board_t));

	assert(fen->board);

	memset(fen->board, 0, sizeof(board_t));
	memset(fen->board->squares, INVALID_PIECE, 64 * sizeof(u8));

	fen->alloc = TRUE;

	rc = parse_rank(fen, str, len);

	return rc;
}


void fen_destroy(fen_state_t *fen)
{
	assert(fen);
	
	fen->state = FEN_RANK_BEGIN;
	fen->cur_rank = R1;
	fen->read_pos = 0;

	free(fen->board);

	fen->alloc = FALSE;
}
