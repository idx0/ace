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
/*
 * Large portions of this file are directly adapted from Scid (Shane's Chess
 * Information Database) files pgnparse.h/cpp. Those files are distributed
 * under the GNU General Public License and have the following copyright.
 *
 * Copyright (c) 2001-2003  Shane Hudson.  All rights reserved.
 * Shane Hudson (sgh@users.sourceforge.net)
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "ace_types.h"

#define INCLUDE_ELO

/**
 * pgn_node_t is a move tree node built from a pgn game database.
 * the sat structure can be written to and loaded from disk in
 * depth-first-search order
 */
typedef struct _pgn_node
{
	struct _pgn_node* parent;	/* pointer to parent node (NULL if root) */
	struct _pgn_node* eldest;	/* pointer to eldest child node (NULL if leaf) */
	struct _pgn_node* sibling;	/* pointer next eldest sibling */

	u16 ply;					/* ply count (depth), calculated */

	struct __sat {
		/* pgn values */
		u32 move		: 16;	/* 6 bits for rank, 6 for file, 4 for kind
								 * (capture, en pas., promotion, castle) */
		u32 piece		: 4;	/* 1 bit for color, 3 for piece value */
		u32 nchild		: 6;	/* hopefully no position has more than 64
								 * moves in it, otherwise we will only store the
								 * 64 most common moves. */
		u32 reserved	: 6;

		u32 tag			: 16;
		u32 flags		: 16;

		/* statistical values */
		u32 points;				/* 10 * points earned by white in this line */
		u32 count;				/* number of times this node was visited */

		/* elo */
#ifdef INCLUDE_ELO
		u32 elow		: 16;	/* cumulative moving average of white elo */
		u32 elob		: 16;	/* cumulative moving average of black elo */
#endif
	} sat;

} pgn_node_t;

#ifdef INCLUDE_ELO
#define NODE_LEN 18
#else
#define NODE_LEN 16
#endif


#define FLAG_NONE			0x0000
#define FLAG_START			0x0001	/* move was played from a starting position */
#define FLAG_BOOK			0x0002	/* seen by opening/ending book */
#define FLAG_LOSING			0x0004	/* move was losing, lost next move */
#define FLAG_CHECK			0x0008	/* move is a check */
#define FLAG_DBLCHECK		0x0010	/* move has ++ */
#define FLAG_MATE			0x0020	/* move is a checkmate */
#define FLAG_EXCLAM			0x0040	/* move has ! */
#define FLAG_DBLEXCLAM		0x0080	/* move has !! */
#define FLAG_QUESTION		0x0100	/* move has ? */
#define FLAG_BLUNDER		0x0200	/* move has ?? */
#define FLAG_DUBIOUS		0x0400	/* move has ?! */
#define FLAG_ADVWHITE		0x0800	/* move has +/= or +/- */
#define FLAG_ADVBLACK		0x1000	/* move has =/+ or -/+ */
#define FLAG_EVEN			0x2000	/* move has = */
#define FLAG_RESERVED		0x4000
#define FLAG_LAST			0x8000	/* flag max */


typedef struct _pgn_tree
{
	pgn_node_t* root;
	u32 count;		/* total count of moves */
	u16 depth;		/* depth of longest line */
} pgn_tree_t;

/**
 * Writes tree node data to fp in depth-first search order.
 */
extern void pgntree_write_dfs(pgn_tree_t* tree, const char *filename);

extern void pgntree_add(pgn_tree_t* tree, const game_t* game);

/******************************** PGN Buffer  ********************************/

#define PGN_MAX_UNGET	16

#define PGN_LINEBREAK		'\n'
#define PGN_CARRIAGERETURN	'\r'

typedef struct _pgn_buffer
{
	u32 seen;					/* bytes seen */
	u16 lines;					/* line count */

	char unget[PGN_MAX_UNGET];	/* unget buffer */
	u16 ugcnt;					/* unget count */

	FILE* infile;				/* if reading from a file, this is non-NULL */
	char* inbuf;				/* if reading from an address, this is non-NULL */
} pgn_buffer_t;


extern char pgn_buffer_get(pgn_buffer_t* buffer);
extern void pgn_buffer_line(pgn_buffer_t* buffer,
							char* outbuf,
							u16 outsize);

/* returns non-zero if buffer is at EOF */
extern int pgn_buffer_eof(pgn_buffer_t* buffer);

extern void pgn_buffer_unget(pgn_buffer_t* buffer, const char ch);

extern void pgn_buffer_free(pgn_buffer_t* buffer);

extern pgn_buffer_t* pgn_buffer_new_file(const char* filename);
extern pgn_buffer_t* pgn_buffer_new_addr(void* ptr);
extern pgn_buffer_t* pgn_buffer_new_block(u32 size);


/******************************** PGN Tokens  ********************************/

typedef enum pgn_token
{
	TOKEN_FIRST = 0,

	TOKEN_EOI		= TOKEN_FIRST,	/* end of input */
	TOKEN_INVALID,					/* invalid or unknown token */
	TOKEN_MOVENUM	= 3,			/* move number */
	TOKEN_IGNORE,					/* ignored text */

	/* move tokens */
	TOKEN_MOVE_PAWN			= 10,
	TOKEN_MOVE_PROMOTE,
	TOKEN_MOVE_PIECE,
	TOKEN_MOVE_CASTLE,
	TOKEN_MOVE_CASTLE_LONG,
	TOKEN_MOVE_NULL,

	/* tag tokens */
	TOKEN_TAG				= 20,
	TOKEN_TAG_END,

	/* result tokens */
	TOKEN_RESULT_UNKNOWN	= 40,
	TOKEN_RESULT_WHITE, 
	TOKEN_RESULT_BLACK,
	TOKEN_RESULT_DRAW,

	/* comments, variations */
	TOKEN_NAG				= 50,	/* $xx */
	TOKEN_SUFFIX,					/* !, ?, !!, ??, !?, ?! */
	TOKEN_CHECK,					/* + */
	TOKEN_MATE,						/* # */
	TOKEN_COMMENT,					/* { ... } */
	TOKEN_LINECOMMENT,				/* %, ; */
	TOKEN_VARSTART,					/* ( ... */
	TOKEN_VAREND,					/* ... ) */
	TOKEN_COMMENTEND,				/* } (misplaced comment end token) */

	TOKEN_LAST,
	TOKEN_COUNT = (TOKEN_LAST - TOKEN_FIRST)
} pgn_token_t;

#define is_tag(t) ((t) == TOKEN_TAG)
#define is_result(t) (((t) >= TOKEN_RESULT_UNKNOWN) && ((t) <= TOKEN_RESULT_DRAW))

/*****************************************************************************/

#define MAX_TOKEN_SIZE	16383

typedef enum _pgn_parsing
{
	PARSE_GARBAGE,
	PARSE_HEADER,
	PARSE_MOVES
} pgn_parsing_t;

typedef struct _pgn
{
	pgn_tree_t tree;
	pgn_buffer_t* buffer;

	game_t game;

	pgn_parsing_t state;
} pgn_t;

/**
 * Returns the next token in the buffer contained within pgn->buffer.
 * the string value of the token is placed in outbuf
 */
extern pgn_token_t pgn_token(pgn_t* pgn, char *outbuf, u16 outlen);

#define PGN_SUCCESS			0
#define PGN_ERROR_BADPTR	-1	/* bad pgn pointer */

#define PGN_ERROR_EOF		-2	/* end of file reached */
#define PGN_ERROR_INVALID	-3	/* invalid character in pgn text */
#define PGN_ERROR_GAME		-4	/* problem parsing current game */
#define PGN_ERROR_MOVE		-5	/* problem parsing move */

/**
 * Parses a the pgn games contained in the pgn buffer.  This function returns 0
 * on success, or some error code on failure.
 */
extern int pgn_parse(pgn_t* pgn);

/* open/close pgn file a pgn file */
extern int pgn_open(pgn_t* pgn, const char* filename);
extern void pgn_close(pgn_t* pgn);

extern void pgn_new_game(pgn_t* pgn);

/**
 * The PGN api can be called like this to load a pgn file and all
 * of its games.  If a call to pgn_parse is successful, then pgn.game
 * will contain the game that was parsed.
 *
 * pgn_open(&pgn, PGN_FILE);
 * while (pgn_parse(&pgn) == PGN_SUCCESS) {
 *     // we can use our board for whatever here!
 *     pgn_new_game(&pgn);
 * }
 * pgn_close(&pgn);
 *
 */


/* test function */
extern void pgn_test();

extern void print_game(undolist_t *ul);
extern void pgntree_print(pgn_tree_t* tree, game_t* game);

#ifdef __cplusplus
}
#endif
