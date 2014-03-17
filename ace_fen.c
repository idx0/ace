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
#include <stdio.h>

#include "ace_fen.h"
#include "ace_global.h"
#include "ace_zobrist.h"

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


static piece_t from_char(const char c)
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


static int read_delimiting_space(fen_state_t *fen, const char *str, size_t len, int rc)
{
	static const char SPACE = ' ';

	/* read deliminating space */
	if ((rc == FEN_SUCCESS) && (fen->read_pos < len)) {
		if (str[fen->read_pos] == SPACE) {
			fen->read_pos++;
		} else {
			rc = ERROR_FEN_INVALID_CHAR;
		}
	}

	return rc;
}


static int parse_rank(fen_state_t *fen, const char *str, size_t len)
{
	static const char TERM = '/';
	static const char SPACE = ' ';
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

			if ((c == TERM) || (c == SPACE)) {	/* terminal symbol */
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
				file_cnt += (c - '0');
			} else if (iswhite(c) || isblack(c)) {
				/* TODO: add piece */
				p = from_char(c);
				index = from_rank_file(7 - rank_cnt, file_cnt);

				if (is_valid_index(index)) {
					add_piece(fen->board, index, p);
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


static int parse_side(fen_state_t *fen, const char *str, size_t len)
{
	static const char TERM = ' ';
	int rc = FEN_SUCCESS;
	int readcolor = FALSE;
	char c;

	while ((fen->read_pos < len) && (rc == FEN_SUCCESS) && (!readcolor)) {
		c = str[fen->read_pos];

		if (c == TERM) {
			if (!readcolor) {
				rc = ERROR_FEN_INVALID_CHAR;
			}
		} else if (c == 'w') {
			fen->board->side = WHITE;
			readcolor = TRUE;
		} else if (c == 'b') {
			fen->board->side = BLACK;
			readcolor = TRUE;

			fen->board->key ^= hash_side;
		}

		fen->read_pos++;
	}

	return read_delimiting_space(fen, str, len, rc);
}


static int parse_castle(fen_state_t *fen, const char *str, size_t len)
{
	static const char TERM = ' ';
	int rc = FEN_SUCCESS;
	int castval = FALSE;
	char c;

	fen->board->castle = 0xf0;

	while ((fen->read_pos < len) && (rc == FEN_SUCCESS) && (!castval)) {
		c = str[fen->read_pos];

		if (c == TERM) {
			if (fen->board->castle <= 0x0f) {
				castval = TRUE;
				fen->board->key ^= hash_castle[fen->board->castle];
			} else {
				rc = ERROR_FEN_INVALID_CASTLE;
			}
		} else if (c == 'K') {
			/* white castle kingside */
			fen->board->castle &= 0x0f;
			fen->board->castle |= WK;
		} else if (c == 'k') {
			/* black castle kingside */
			fen->board->castle &= 0x0f;
			fen->board->castle |= BK;
		} else if (c == 'Q') {
			/* white castle queenside */
			fen->board->castle &= 0x0f;
			fen->board->castle |= WQ;
		} else if (c == 'q') {
			/* black castle queenside */
			fen->board->castle &= 0x0f;
			fen->board->castle |= BQ;
		} else if (c == '-') {
			/* no castling */
			fen->board->castle = 0;
			fen->board->key ^= hash_castle[0];
			castval = TRUE;
		} else {
			rc = ERROR_FEN_INVALID_CHAR;
		}

		fen->read_pos++;
	}

	if (fen->board->castle == 0) {
		rc = read_delimiting_space(fen, str, len, rc);
	}

	return rc;
}


static int parse_enpas(fen_state_t *fen, const char *str, size_t len)
{
	static const char TERM = ' ';
	static const u8 MAGIC_RANK = 0x78;

	int rc = FEN_SUCCESS;
	int readep = FALSE;
	char c;

	fen->board->enpas = INVALID_SQUARE;

	while ((fen->read_pos < len) && (rc == FEN_SUCCESS) && (!readep)) {
		c = str[fen->read_pos];

		if (c == TERM) {
			if ((!readep) || (!is_valid_index(fen->board->enpas) &&
				              (fen->board->enpas != INVALID_SQUARE))) {
				rc = ERROR_FEN_ILLEGAL_ENPAS;
				fen->board->enpas = INVALID_SQUARE;
			}
		} else if (c == '-') {
			readep = TRUE;
		} else if (isfile(c)) {
			fen->board->enpas = from_rank_file(0, (c - 'a'));
			fen->board->enpas |= MAGIC_RANK;
		} else if ((c == '3') || (c == '6')) {
			fen->board->enpas = from_rank_file((c - '0'),
											   file(fen->board->enpas));
			fen->board->key ^= hash_enpas[file(fen->board->enpas)];
			readep = TRUE;
		} else {
			rc = ERROR_FEN_ILLEGAL_ENPAS;
		}

		fen->read_pos++;
	}

	return read_delimiting_space(fen, str, len, rc);
}


static int parse_moves(fen_state_t *fen, const char *str, size_t len)
{
	int ret, plies, moves;
	int rc = FEN_SUCCESS;

	fen->board->plies = 0;
	fen->board->moves = 1;

	if (fen->read_pos < len) {
		ret = sscanf(str + fen->read_pos, "%d %d", &plies, &moves);

		if (ret == 2) {
			if ((plies < 0) || (plies > 100)) {
				rc = ERROR_FEN_INVALID_CLOCK;
			} else if (moves <= 0) {
				rc = ERROR_FEN_INVALID_MOVE;
			} else {
				fen->board->plies = plies;
				fen->board->moves = moves;
			}
		} else {
			rc = ERROR_FEN_INVALID_CHAR;
		}
	} else {
		/* we will allow move and half-move data to be omitted */
		/* rc = ERROR_FEN_INVALID_MOVE; */
	}

	return rc;
}


int fen_init(fen_state_t *fen)
{
	int rc = FEN_SUCCESS;

	assert(fen);

	fen->state = FEN_RANK_BEGIN;
	fen->cur_rank = R1;
	fen->read_pos = 0;

	fen->board = (board_t*)malloc(sizeof(board_t));

	if (fen->board) {
		memset(fen->board, 0, sizeof(board_t));
		memset(fen->board->pos.squares, INVALID_PIECE, 64 * sizeof(u8));

		fen->alloc = TRUE;
	} else {
		rc = ERROR_FEN_MEMORY;
	}

	return rc;
}


int fen_parse(fen_state_t *fen, const char *str, size_t len)
{
	int rc = FEN_SUCCESS;

	assert(fen);
	assert(fen->board);

	fen->state = FEN_RANK_BEGIN;
	fen->cur_rank = R1;
	fen->read_pos = 0;

	memset(fen->board, 0, sizeof(board_t));
	memset(fen->board->pos.squares, INVALID_PIECE, 64 * sizeof(u8));

	/* fail chain */
	if ((rc = parse_rank(fen, str, len)) != FEN_SUCCESS) {
		fprintf(stderr, "fen_parse: error %d in rank at %ld `%c'\n",
			rc,
			fen->read_pos,
			(fen->read_pos < len ? str[fen->read_pos] : str[len - 1]));
	} else if ((rc = parse_side(fen, str, len)) != FEN_SUCCESS) {
		fprintf(stderr, "fen_parse: error %d in side at %ld `%c'\n",
			rc,
			fen->read_pos,
			(fen->read_pos < len ? str[fen->read_pos] : str[len - 1]));
	} else if ((rc = parse_castle(fen, str, len)) != FEN_SUCCESS) {
		fprintf(stderr, "fen_parse: error %d in castle at %ld `%c'\n",
			rc,
			fen->read_pos,
			(fen->read_pos < len ? str[fen->read_pos] : str[len - 1]));
	} else if ((rc = parse_enpas(fen, str, len)) != FEN_SUCCESS) {
		fprintf(stderr, "fen_parse: error %d in enpas at %ld `%c'\n",
			rc,
			fen->read_pos,
			(fen->read_pos < len ? str[fen->read_pos] : str[len - 1]));
	} else if ((rc = parse_moves(fen, str, len)) != FEN_SUCCESS) {
		fprintf(stderr, "fen_parse: error %d in moves at %ld `%c'\n",
			rc,
			fen->read_pos,
			(fen->read_pos < len ? str[fen->read_pos] : str[len - 1]));
	}

	return rc;
}


void fen_use_ptr(fen_state_t *fen, board_t *boardptr)
{
	assert(fen);
	assert(boardptr);

	if (fen->alloc) {
		free(fen->board);
		fen->alloc = FALSE;
	}

	fen->board = boardptr;
}


void fen_destroy(fen_state_t *fen)
{
	assert(fen);
	
	fen->state = FEN_RANK_BEGIN;
	fen->cur_rank = R1;
	fen->read_pos = 0;

	if (fen->alloc) {
		free(fen->board);
	}

	fen->alloc = FALSE;
}
