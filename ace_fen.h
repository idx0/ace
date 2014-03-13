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

#pragma once

#include <stdlib.h>

#include "ace_types.h"

#define FEN_SUCCESS					0

/* a character is invalid in the string */
#define ERROR_FEN_INVALID_CHAR		1
/* a value for a rank is invalid */
#define ERROR_FEN_INVALID_RANK		2
/* a value for a file is invalid */
#define ERROR_FEN_INVALID_FILE		3
/* the halfmove clock valud is invalid (greater than 100) */
#define ERROR_FEN_INVALID_CLOCK		4
/* the fullmove value is invalid (namely, not set or 0) */
#define ERROR_FEN_INVALID_MOVE		5
/* the given en passant square is illegal */
#define ERROR_FEN_ILLEGAL_ENPAS		6
/* the given rank information is incomplete */
#define ERROR_FEN_INCOMPLETE_RANK	7


enum fen_parse_state { FEN_RANK_BEGIN = 0, FEN_RANK_END, FEN_SIDE,
					   FEN_CASTLING, FEN_ENPAS, FEN_HALF, FEN_FULL, FEN_COMPLETE };

/* returns TRUE if c is a valid value for rank */
int isrank(const char c);

/* returns TRUE if c is a valid value for file */
int isfile(const char c);

/* returns TRUE if c is non-zero (1-9) */
int isnonzero(const char c);

/* returns TRUE if c is zero (0) */
int iszero(const char c);

/* returns TRUE if c is a white piece */
int iswhite(const char c);

/* returns TRUE if c is a black piece */
int isblack(const char c);

/* this structure is used to keep fen state */
typedef struct fen_state {

	enum fen_parse_state state;
	
	board_rank_t		 cur_rank;
	size_t				 read_pos;
	int 				 alloc;

	board_t* board; /* TODO */
} fen_state_t;

/* returns a struct piece from the given character */
piece_t from_char(const char c, const board_rank_t rank, const board_file_t file);

int parse_rank(fen_state_t *fen, const char *str, size_t len);

/**
 * Initializes fen and parses a string in Forsyth-Edwards Notation
 * @param fen FEN state structure
 * @param str FEN string text
 * @param len length of fen
 * @return FEN_SUCCESS if successful, some ERROR_FEN_X if unsuccessful
 */
extern int fen_init(fen_state_t *fen, const char *str, size_t len);

/**
 * Frees memory allocted in fen
 * @param fen FEN state structure
 */
extern void fen_destroy(fen_state_t *fen);
