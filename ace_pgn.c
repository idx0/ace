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

#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "ace_pgn.h"
#include "ace_columns.h"

#include "ace_intrin.h"
#include "ace_global.h"
#include "ace_str.h"
#include "ace_fen.h"


pgn_node_t* pgnnode_alloc()
{
	pgn_node_t *node = (pgn_node_t*)malloc(sizeof(pgn_node_t));
	node->ply = 0;
	node->sibling = NULL;
	node->eldest = NULL;
	node->parent = NULL;
	memset(&node->sat, 0, NODE_LEN);

	return node;
}

/* frees a node and all children recursively - it does not fix sibling pointers of
 * of node
 */
static void pgnnode_free_branch(pgn_node_t *node)
{
	pgn_node_t *child, *tmp;

	if (node) {
		child = node->eldest;

		free(node);

		while (child) {
			tmp = child;
			child = child->sibling;

			pgnnode_free_branch(tmp);
		}
	}
}


void pgnnode_free(pgn_node_t* node)
{
	pgn_node_t *cur, *prev;

	/* fix parent, sibling pointers */
	if (node->parent) {
		cur = node->parent;

		/* check siblings */
		if (cur->eldest == node) {
			cur->eldest = node->sibling;
		} else if (cur->eldest) {
			prev = cur->eldest;
			cur = prev->sibling;

			while (cur) {
				if (cur == node) {
					prev->sibling = cur->sibling;
					break;
				}

				prev = cur;
				cur = cur->sibling;
			}
		}
	}

	/* free children */
	pgnnode_free_branch(node);
}


static void pgnnode_write_dfs(FILE *fp, pgn_node_t* node)
{
	u16 nc = 0;
	pgn_node_t* child;

	if (node) {
		// write root
		fwrite(&node->sat, NODE_LEN, 1, fp);

		// write children
		child = node->eldest;

		while (child) {
			nc++;
			pgnnode_write_dfs(fp, child);

			child = child->sibling;
		}

		if (nc != node->sat.nchild) {
			// error
		}
	}
}


void pgntree_write_dfs(pgn_tree_t* tree, const char *filename)
{
	FILE *fp;
	
	if ((!tree) || (!tree->root)) return;
	
	fp = fopen(filename, "w");

	if (fp) {

		pgnnode_write_dfs(fp, tree->root);

		fclose(fp);
	}
}


void pgntree_add(pgn_tree_t* tree, const game_t* game)
{
	u16 i;
	pgn_node_t *parent, *child, *youngest;
	move_t m;

	board_t board;
	game_t g;

	if ((!tree) || (!game)) return;
	if (!tree->root) return;

	parent = tree->root;

	g.board = &board;
	new_game(&g);

	for (i = 0; i < game->undo.count; i++) {
		child = parent->eldest;
		youngest = child;
		m = game->undo.undo[i].move;

		while (child) {
			if (child->sat.move == m) {
				/* found */
				break;
			}

			youngest = child;
			child = child->sibling;
		}

		/* add child */
		if (!child) {
			if (youngest) {
				youngest->sibling = pgnnode_alloc();
				child = youngest->sibling;
			} else {
				parent->eldest = pgnnode_alloc();
				child = parent->eldest;
			}

			parent->sat.nchild++;

			child->parent = parent;
			child->ply = parent->ply + 1;
			child->sat.move = m;
			child->sat.piece = game->board->pos.squares[move_from(m)];

			if (parent == tree->root) {
				child->sat.flags |= FLAG_START;
			}
		}

		/* update */
		child->sat.count++;

		if (game->board->side == WHITE) {
			child->sat.points += (game->result == WIN_WHITE ? 10 : (game->result == DRAW ? 5 : 0));
		} else {
			child->sat.points += (game->result == WIN_BLACK ? 10 : (game->result == DRAW ? 5 : 0));
		}

		do_move(g.board, &g.undo, m);
		parent = child;
	}
}


int pgn_buffer_eof(pgn_buffer_t* buffer)
{
	if (!buffer) return 1;
	
	if (buffer->infile) return feof(buffer->infile);
	if (buffer->inbuf) return (!(*buffer->inbuf));

	return 1;
}


char pgn_buffer_get(pgn_buffer_t* buffer)
{
	char ch = 0;
	if (!buffer) return ch;

	buffer->seen++;

	if (buffer->ugcnt > 0) {
		ch = buffer->unget[--buffer->ugcnt];
	} else if (buffer->infile) {
		ch = fgetc(buffer->infile);
	} else if (buffer->inbuf) {
		ch = *buffer->inbuf;
		if (ch) buffer->inbuf++;
	}

	if (ch == PGN_LINEBREAK) buffer->lines++;

	return ch;
}

void pgn_buffer_line(pgn_buffer_t* buffer,
					 char* outbuf,
					 u16 outsize)
{
	char ch;

	if ((!outsize) || (!buffer)) return;

	while (1) {
		ch = pgn_buffer_get(buffer);

		if ((ch == EOF) || (ch == 0) || (ch == PGN_LINEBREAK)) break;

		if (ch == PGN_CARRIAGERETURN) {
			ch = pgn_buffer_get(buffer);
			if (ch != PGN_LINEBREAK) {
				pgn_buffer_unget(buffer, ch);
			}
			break;
		}

		outsize--;
		if (outsize == 0) break;

		*outbuf++ = ch;
	}

	*outbuf = 0;
}


void pgn_buffer_unget(pgn_buffer_t* buffer, const char ch)
{
	if (!buffer) return;
	if (buffer->ugcnt == PGN_MAX_UNGET) return;

	buffer->unget[buffer->ugcnt++] = ch;
	buffer->seen--;

	if (ch == PGN_LINEBREAK) buffer->lines--;
}

void pgn_buffer_free(pgn_buffer_t* buffer)
{
	if (buffer) {
		if (buffer->infile) fclose(buffer->infile);
		if (buffer->inbuf) free(buffer->inbuf);

		free(buffer);
	}
}


pgn_buffer_t* pgn_buffer_new_file(const char* filename)
{
	pgn_buffer_t* buffer = (pgn_buffer_t*)malloc(sizeof(pgn_buffer_t));

	if (buffer) {
		buffer->infile = fopen(filename, "r");
		buffer->inbuf = NULL;

		buffer->lines = 0;
		buffer->seen = 0;
		buffer->ugcnt = 0;
		memset(buffer->unget, 0, PGN_MAX_UNGET);
	}

	return buffer;
}


pgn_buffer_t* pgn_buffer_new_addr(void* ptr)
{
	pgn_buffer_t* buffer = (pgn_buffer_t*)malloc(sizeof(pgn_buffer_t));

	if (buffer) {
		buffer->infile = NULL;
		buffer->inbuf = (char*)ptr;

		buffer->lines = 0;
		buffer->seen = 0;
		buffer->ugcnt = 0;
		memset(buffer->unget, 0, PGN_MAX_UNGET);
	}

	return buffer;
}


pgn_buffer_t* pgn_buffer_new_block(u32 size)
{
	pgn_buffer_t* buffer = (pgn_buffer_t*)malloc(sizeof(pgn_buffer_t));

	if (buffer) {
		buffer->infile = NULL;
		buffer->inbuf = (char*)malloc(size);

		buffer->lines = 0;
		buffer->seen = 0;
		buffer->ugcnt = 0;
		memset(buffer->unget, 0, PGN_MAX_UNGET);
	}

	return buffer;
}

/*****************************************************************************/

/* rest of word */
static u16 pgn_word(pgn_buffer_t* buffer, char* outbuf, u16 remaining)
{
	char ch = pgn_buffer_get(buffer);
	u16 cnt = 0;

	while ((cnt < remaining) && (!isspace(ch)) &&
		   (ch != '.') && (ch != ')') &&
		   (ch != EOF) && (ch != 0)) {
		outbuf[cnt++] = ch;
		ch = pgn_buffer_get(buffer);
	}

	pgn_buffer_unget(buffer, ch);
	outbuf[cnt] = 0;

	return cnt;
}


/* rest of word including dots */
static u16 pgn_word_dots(pgn_buffer_t* buffer, char* outbuf, u16 remaining)
{
	char ch = pgn_buffer_get(buffer);
	u16 cnt = 0;

	while ((cnt < remaining) &&
		   (!isspace(ch)) && (ch != ')') &&
		   (ch != EOF) && (ch != 0)) {
		outbuf[cnt++] = ch;
		ch = pgn_buffer_get(buffer);
	}

	pgn_buffer_unget(buffer, ch);
	outbuf[cnt] = 0;

	return cnt;
}


/* rest of word including alphabetic characters only */
static u16 pgn_word_alpha(pgn_buffer_t* buffer, char* outbuf, u16 remaining)
{
	char ch = pgn_buffer_get(buffer);
	u16 cnt = 0;

	while ((cnt < remaining) && (isalpha(ch))) {
		outbuf[cnt++] = ch;
		ch = pgn_buffer_get(buffer);
	}

	pgn_buffer_unget(buffer, ch);
	outbuf[cnt] = 0;

	return cnt;
}

/*****************************************************************************/

static pgn_token_t game_token_digit(char *buffer, u16 buflen)
{
	int alldigits = 1;
	char *s = buffer;

	while (*s) {
		if (!isdigit(*s)) {
			alldigits = 0;
			break;
		}
		s++;
	}

	if (*buffer == '0') {
		if (str_equal(buffer, "0-1")) return TOKEN_RESULT_BLACK;
		if (str_equal(buffer, "0:1")) return TOKEN_RESULT_BLACK;
		if (str_begin("0-0-0", buffer)) return TOKEN_MOVE_CASTLE_LONG;
		if (str_begin("000", buffer)) return TOKEN_MOVE_CASTLE_LONG;
		if (str_begin("0-0", buffer)) return TOKEN_MOVE_CASTLE;
		if (str_begin("00", buffer)) return TOKEN_MOVE_CASTLE;
	} else if (*buffer == '1') {
		if (str_equal(buffer, "1-0")) return TOKEN_RESULT_WHITE;
		if (str_equal(buffer, "1:0")) return TOKEN_RESULT_WHITE;
		if (str_equal(buffer, "1/2")) return TOKEN_RESULT_DRAW;
		if (str_equal(buffer, "1/2-1/2")) return TOKEN_RESULT_DRAW;
		if (str_equal(buffer, "1/2:1/2")) return TOKEN_RESULT_DRAW;
	}

	if (alldigits) {
		return TOKEN_MOVENUM;
	}

	return TOKEN_INVALID;
}

/*****************************************************************************/

#define buf_set(buf, ch, i, r) (buf)[(i)] = (ch); (i)++; (r)--; (buf)[(i)] = 0;

static pgn_token_t game_token_suffix(pgn_buffer_t* buffer, char *outbuf, u16 remaining)
{
	char first = *(outbuf - 1);
	u16 bufpos = 0;
	char ch;

	if ((first == '!') || (first == '?')) {
		ch = pgn_buffer_get(buffer);

		while ((ch == '!') || (ch == '?')) {
			buf_set(outbuf, ch, bufpos, remaining);
			ch = pgn_buffer_get(buffer);
		}

		pgn_buffer_unget(buffer, ch);
	} else {
		/* +/-, +/=, etc. */
		pgn_word(buffer, outbuf, remaining);
	}

	return TOKEN_SUFFIX;
}


static pgn_token_t game_token_castle(pgn_buffer_t* buffer, char *outbuf, u16 remaining)
{
	int ohcnt = 1;
	u16 bufpos = 0;
	char ch, next;

	while (remaining) {
		ch = pgn_buffer_get(buffer);

		if ((ch == 'O') || (ch == 'o') || (ch == '0')) {
			ohcnt++;
			buf_set(outbuf, ch, bufpos, remaining);
			continue;
		}

		if (ch == '-') {
			next = pgn_buffer_get(buffer);
			pgn_buffer_unget(buffer, next);

			if ((next == '+') || (next == '/')) {
				/* not from castling */
				pgn_buffer_unget(buffer, ch);
				break;
			}

			buf_set(outbuf, ch, bufpos, remaining);
			continue;
		}

		if (isspace(ch) ||
			(ch == '+') || (ch == '#') || (ch == '!') || (ch == '?') ||
			(ch == ')') || (ch == EOF) || (ch == 0) || (ch == '=')) {
			pgn_buffer_unget(buffer, ch);

			switch (ohcnt) {
			case 2: return TOKEN_MOVE_CASTLE;
			case 3: return TOKEN_MOVE_CASTLE_LONG;
			default: return TOKEN_INVALID;
			}
		}
		break;
	}

	pgn_word_dots(buffer, outbuf + bufpos, remaining - bufpos);
	return TOKEN_INVALID;
}

static pgn_token_t game_token_move(pgn_buffer_t* buffer, char *outbuf, u16 remaining)
{
	int len = 1;
	u16 bufpos = 0;
	char ch, next;

	while (remaining) {
		ch = pgn_buffer_get(buffer);

		if (isspace(ch) ||
			(ch == '+') || (ch == '#') || (ch == '!') || (ch == '?') ||
			(ch == ')') || (ch == EOF) || (ch == 0) || (ch == '=')) {
			pgn_buffer_unget(buffer, ch);
			return ((len == 1) ? TOKEN_SUFFIX : TOKEN_MOVE_PIECE);
		}

		if ((isrank(ch)) || (isfile(ch))) {
			buf_set(outbuf, ch, bufpos, remaining);
			len++;
			continue;
		}

		if (ch == '-') {
			/* look for -+, -/+ (and ignoree it) */
			next = pgn_buffer_get(buffer);
			pgn_buffer_unget(buffer, next);

			if ((next == '+') || (next == '/')) {
				pgn_buffer_unget(buffer, ch);
				break;
			}

			len++;
			continue;
		}

		if ((ch == 'x') || (ch == ':')) {
			buf_set(outbuf, 'x', bufpos, remaining);
			len++;
			continue;
		}
	}

	return TOKEN_INVALID;
}

static pgn_token_t game_token_pawn(pgn_buffer_t* buffer, char *outbuf, u16 remaining)
{
	char ch, next;
	u16 bufpos = 0;
	int digits = 0, pawn = 0;

	/* check ep or e.p. */
	if (*(outbuf - 1) == 'e') {
		ch = pgn_buffer_get(buffer);
		pgn_buffer_unget(buffer, ch);

		if ((ch == 'p') || (ch == '.')) {
			pgn_word_dots(buffer, outbuf, remaining);
			return TOKEN_IGNORE;
		}
	}

	while (remaining) {
		ch = pgn_buffer_get(buffer);

		if (isspace(ch)) {
			pgn_buffer_unget(buffer, ch);
			return TOKEN_MOVE_PAWN;
		}

		if (digits) {
			/* look for ep, e.p. after digit */
			if (ch == 'e') {
				next = pgn_buffer_get(buffer);
				pgn_buffer_unget(buffer, next);

				if ((next == 'p') || (next == '.')) continue;
			}
			if ((ch == 'p') || (ch == '.')) continue;
		}

		if (isrank(ch)) {
			digits = 1;
			buf_set(outbuf, ch, bufpos, remaining);
			continue;
		}

		if ((isfile(ch)) && (!pawn)) {
			pawn = 1;
			buf_set(outbuf, ch, bufpos, remaining);
			continue;
		}

		if (ch == '-') {
			/* look for -+, -/+ (and ignoree it) */
			next = pgn_buffer_get(buffer);
			pgn_buffer_unget(buffer, next);

			if ((next == '+') || (next == '/')) {
				pgn_buffer_unget(buffer, ch);
				return TOKEN_MOVE_PAWN;
			}
			continue;
		}

		/* capture */
		if ((ch == 'x') || (ch == ':')) {
			buf_set(outbuf, 'x', bufpos, remaining);
			continue;
		}

		if (ch == '=') {
			ch = pgn_buffer_get(buffer);

			/* promotion */
			if (ismaterial(ch)) {
				buf_set(outbuf, '=', bufpos, remaining);
				buf_set(outbuf, toupper(ch), bufpos, remaining);
				return TOKEN_MOVE_PROMOTE;
			} else {
				/* not promotion, could be equality notation */
				pgn_buffer_unget(buffer, ch);
				pgn_buffer_unget(buffer, '=');
				return TOKEN_MOVE_PAWN;
			}
		}

		/* promotions without '=' symbol */
		if (ismaterial(ch)) {
			buf_set(outbuf, '=', bufpos, remaining);
			buf_set(outbuf, toupper(ch), bufpos, remaining);
			return TOKEN_MOVE_PROMOTE;
		}

		if ((ch == '+') || (ch == '#') || (ch == '!') || (ch == '?') ||
			(ch == ')') || (ch == EOF) || (ch == 0)) {
			pgn_buffer_unget(buffer, ch);
			return TOKEN_MOVE_PAWN;
		}
	}

	return TOKEN_INVALID;
}


static pgn_token_t game_token(pgn_buffer_t* buffer, char* outbuf, u16 outlen)
{
	char *buf = outbuf;
	u16 bufpos = 0;
	char ch = pgn_buffer_get(buffer), next;

	/* end of file */
	if ((ch == EOF) || (ch == 0)) return TOKEN_EOI;

	/* remove leading spaces */
	while ((isspace(ch)) || (ch == '.')) {
		ch = pgn_buffer_get(buffer);
		if ((ch == EOF) || (ch == 0)) return TOKEN_EOI;
	}

	buf[bufpos++] = ch;

	if (isdigit(ch)) {
		bufpos += pgn_word(buffer, buf + bufpos, outlen - bufpos);

		return game_token_digit(buf, bufpos);
	} else if (isfile(ch)) {
		/* pawn move */
		return game_token_pawn(buffer, buf + bufpos, outlen - bufpos);
	} else if (ch == 'P') {
		/* pawn move, ignore P */
		bufpos = 0;
		buf[bufpos++] = pgn_buffer_get(buffer);

		return game_token_pawn(buffer, buf + bufpos, outlen - bufpos);
	} else if (iswhite(ch)) {
		/* non-pawn move */

		return game_token_move(buffer, buf + bufpos, outlen - bufpos);
	} else if ((ch == 'O') || (ch == 'o')) {
		/* castle */
		return game_token_castle(buffer, buf + bufpos, outlen - bufpos);
	} else if (ch == 'n') {
		/* null move? */
		bufpos += pgn_word(buffer, buf + bufpos, outlen - bufpos);
		if (str_equal(buf, "null")) {
			return TOKEN_MOVE_NULL;
		}
		return TOKEN_INVALID;
	} else if ((ch == ';') || (ch == '%')) {
		/* check for comments */
		pgn_buffer_line(buffer, buf + bufpos, outlen - bufpos);
		return TOKEN_LINECOMMENT;
	} else if (ch == '{') {
		return TOKEN_COMMENT;
	} else if (ch == '}') {
		return TOKEN_COMMENTEND;
	} else if (ch == '(') {
		return TOKEN_VARSTART;
	} else if (ch == ')') {
		return TOKEN_VAREND;
	} else if ((ch == '!') || (ch == '?') ||
			   (ch == '=') || (ch == '-')) {
		next = pgn_buffer_get(buffer);

		if ((ch == '-') && (next == '-')) {
			buf[bufpos++] = '-';
			buf[bufpos++] = 0;

			return TOKEN_MOVE_NULL;
		}

		pgn_buffer_unget(buffer, next);

		return game_token_suffix(buffer, buf + bufpos, outlen - bufpos);
	} else if (ch == '$') {
		pgn_word(buffer, buf + bufpos, outlen - bufpos);
		return TOKEN_NAG;
	} else if ((ch == '+') || (ch == '#')) {
		next = pgn_buffer_get(buffer);

		/* double check */
		if ((ch == '+') && (next == '+')) { return TOKEN_CHECK; }

		pgn_buffer_unget(buffer, next);

		if ((isspace(next)) || (next == '!') || (next == '?') ||
			(next == '$') || (next == ')') || (next == EOF) || (next ==	 0)) {
			if (ch == '+') return TOKEN_CHECK;
			else return TOKEN_MATE;
		}

		return game_token_suffix(buffer, buf + bufpos, outlen - bufpos);
	} else if (ch == '*') {
		next = pgn_buffer_get(buffer);
		pgn_buffer_unget(buffer, next);

		if ((isspace(next)) ||
			(next == '.') || (next == ')') ||
			(next == EOF) || (next == 0)) {
			return TOKEN_RESULT_UNKNOWN;
		}

		return TOKEN_INVALID;
	} else if (ch == '[') {
		pgn_buffer_unget(buffer, ch);
		return TOKEN_TAG;
	} else if (ch == 'D') {
		pgn_word(buffer, buf + bufpos, outlen - bufpos);
		return TOKEN_NAG;
	} else if (ch == '~') {
		return game_token_suffix(buffer, buf + bufpos, outlen - bufpos);
	} else if (ch == 'Z') {
		next = pgn_buffer_get(buffer);
		if (iszero(next)) {
			return TOKEN_MOVE_NULL;
		}

		pgn_buffer_unget(buffer, next);
	}

	pgn_word_dots(buffer, buf + bufpos, outlen - bufpos);
	return TOKEN_INVALID;
}

/*****************************************************************************/

#define PARSE_MOVE_BUFLEN 255

static int return_error(const char* format, ...)
{
	va_list va;
	va_start(va, format);

	vprintf(format, va);

	va_end(va);

	return PGN_ERROR_MOVE;
}

static int pgn_parse_pawnmove(pgn_t *pgn, movelist_t* ml,
							  char* buffer, u16 buflen, pgn_token_t hint)
{
	u16 i;
	move_t m;
	piece_type_t promo = PAWN;
	board_file_t ff, tf;
	board_rank_t tr;

	if (hint == TOKEN_MOVE_PROMOTE) {
		if (!ismaterial(buffer[buflen - 1])) return return_error("unrecognized promotion: `%s'\n", buffer);

		promo = piece_type(piece_from_char(toupper(buffer[buflen - 1])));

		buflen--;

		/* allow promotion notation both with and without '=' */
		if (buffer[buflen - 1] == '=') { buflen--; }
		if (buflen < 2) return return_error("unrecognized promotion: `%s'\n", buffer);
	} else {
		/* check for coordinate notation (ie e2e4, etc.) */
		if ((buflen >= 4) &&
			(isfile(buffer[0])) && (isrank(buffer[1])) &&
			(isfile(buffer[buflen - 2])) && (isrank(buffer[buflen - 1]))) {
			/* coordinate string */
		}
	}

	/* file */
	if (!isfile(buffer[0])) return return_error("pawn start not file: `%s'\n", buffer);
	ff = file_from_char(buffer[0]);

	/* check for compact capture notation which omits rank */
	if ((buflen == 2) && (isfile(buffer[1]))) {
		tf = file_from_char(buffer[1]);
		/* check the movelist for any pawn captures from the ff to the tf */
		for (i = 0; i < ml->count; i++) {
			m = ml->moves[i];

			if ((is_capture(m)) &&
				(file(move_from(m)) == ff) &&
				(file(move_to(m)) == tf) &&
				((promo == PAWN) || 
					((is_promotion(m)) && (((move_kind(m) & 0x03) + KNIGHT) == promo))) &&
				(pgn->game.board->pos.pawns & (C64(1) << move_from(m)))) {
				/* TODO: pgn_node_t */
				do_move(pgn->game.board, &pgn->game.undo, m);

				return PGN_SUCCESS;
			}
		}

		return return_error("buffer is short capture, but move is illegal: `%s'\n", buffer);
	}

	if ((!isfile(buffer[buflen - 2])) || (!isrank(buffer[buflen - 1]))) {
		return return_error("pawn move without destination: `%s'\n", buffer);
	}

	/* move should be in algebraic form now */
	tf = file_from_char(buffer[buflen - 2]);
	tr = rank_from_char(buffer[buflen - 1]);

	for (i = 0; i < ml->count; i++) {
		m = ml->moves[i];

		if ((file(move_from(m)) == ff) &&
			(move_to(m) == from_rank_file(tr, tf)) &&
			((promo == PAWN) || 
				((is_promotion(m)) && (((move_kind(m) & 0x03) + KNIGHT) == promo))) &&
			(pgn->game.board->pos.pawns & (C64(1) << move_from(m)))) {
			/* TODO: pgn_node_t */
			do_move(pgn->game.board, &pgn->game.undo, m);

			return PGN_SUCCESS;
		}
	}

	return return_error("invalid pawn notation: `%s'\n", buffer);
}


static int pgn_parse_materialmove(pgn_t *pgn, movelist_t* ml, 
								  char* buffer, u16 buflen)
{
	piece_t p;
	u8 to;
	move_t m;
	board_file_t ff = INVALID_FILE;
	board_rank_t fr = INVALID_RANK;
	u16 i;

	p = piece_from_char(toupper(buffer[0]));
	set_piece_color(p, pgn->game.board->side);

	if (piece_type(p) == INVALID_TYPE) return return_error("piece type not recognized: `%s'\n", buffer);
	if ((buflen < 3) || (buflen > 5)) return return_error("move length out-of-range: `%s'\n", buffer);
	if ((!isfile(buffer[buflen - 2])) &&
		(!isrank(buffer[buflen - 1]))) return return_error("move without destination: `%s'\n", buffer);

	to = from_rank_file(rank_from_char(buffer[buflen - 1]),
						file_from_char(buffer[buflen - 2]));

	/* there is ambiguity (Nbd7 for example) */
	if (buflen > 3) {
		i = 1;
		while (i < (buflen - 2)) {
			if (isrank(buffer[i])) fr = rank_from_char(buffer[i]);
			if (isfile(buffer[i])) ff = file_from_char(buffer[i]);
			i++;
		}
	}

	for (i = 0; i < ml->count; i++) {
		m = ml->moves[i];

		/* to squares identical
		 * from square piece is identical
		 * from rank ambiguity is satisfied
		 * from file ambiguity is satisfied
		 */
		if ((move_to(m) == to) &&
			(pgn->game.board->pos.squares[move_from(m)] == p) &&
			((ff == INVALID_FILE) || (file(move_from(m)) == ff)) &&
			((fr == INVALID_RANK) || (rank(move_from(m)) == fr))) {

			if (do_move(pgn->game.board, &pgn->game.undo, m) == TRUE) return PGN_SUCCESS;
		}
	}
	
	return return_error("invalid move notation: `%s'\n", buffer);
}


static int pgn_parse_move(pgn_t *pgn, char* buffer, u16 buflen, pgn_token_t hint)
{
	char buf[PARSE_MOVE_BUFLEN];
	char *s, *p;
	u16 len = 0, i;

	movelist_t ml;
	move_t m;

	/* first, generate all the moves from this board */
	generate_moves(pgn->game.board, &ml, &ml);

	if (hint == TOKEN_MOVE_NULL) {
		/* TODO: pgn_node_t */
		do_null_move(pgn->game.board, &pgn->game.undo);

		return PGN_SUCCESS;
	}

	s = buffer;
	p = buf;

	/* remove 'x' and '-' characters */
	while ((!isspace(*s)) && (*s != 0) &&
		   (len < PARSE_MOVE_BUFLEN)) {
		if (((isalpha(*s)) && (*s != 'x')) ||
			(isdigit(*s)) || (*s == '=')) {
			*p = *s;
			p++;
			len++;
		}
		s++;
	}

	if (len < 2) return return_error("move length out-of-range: `%s'\n", buffer);

	*p = 0;
	s = buf;

	/* pawn moves */
	if ((hint == TOKEN_MOVE_PAWN) || (hint == TOKEN_MOVE_PROMOTE)) {
		return pgn_parse_pawnmove(pgn, &ml, buf, len, hint);
	}

	/* castling */
	if ((hint == TOKEN_MOVE_CASTLE) || (hint == TOKEN_MOVE_CASTLE_LONG)) {
		for (i = 0; i < ml.count; i++) {
			m = ml.moves[i];

			if ((hint == TOKEN_MOVE_CASTLE) &&
				(move_kind(m) == KING_CASTLE)) {
				/* TODO: pgn_node_t */
				do_move(pgn->game.board, &pgn->game.undo, m);

				return PGN_SUCCESS;
			} else if ((hint == TOKEN_MOVE_CASTLE_LONG) &&
					   (move_kind(m) == QUEEN_CASTLE)) {
				/* TODO: pgn_node_t */
				do_move(pgn->game.board, &pgn->game.undo, m);

				return PGN_SUCCESS;
			}
		}

		return return_error("castle illegal: `%s'\n", buffer);
	}

	/* piece move */
	if (hint == TOKEN_MOVE_PIECE) {
		return pgn_parse_materialmove(pgn, &ml, buf, len);
	}

	return return_error("invalid move notation: `%s'\n", buffer);
}


pgn_token_t pgn_token(pgn_t* pgn, char *outbuf, u16 outlen)
{
	if (!pgn) return TOKEN_EOI;
	char* buf, *x;
	u16 bufpos = 0;
	char ch;

	if (pgn->state == PARSE_HEADER) {
		if (pgn_buffer_eof(pgn->buffer)) return TOKEN_EOI;

		buf = outbuf;
		ch = pgn_buffer_get(pgn->buffer);

		buf[bufpos++] = ch;
		buf[bufpos] = 0;

		while ((isspace(ch)) || (ch == '.')) {
			ch = pgn_buffer_get(pgn->buffer);

			buf[bufpos++] = ch;
			buf[bufpos] = 0;
		}

		if ((ch == EOF) || (ch == 0)) {
			return TOKEN_EOI;
		}

		if ((ch == '%') || (ch == ';')) {
			pgn_buffer_line(pgn->buffer, buf + bufpos, outlen - bufpos);
			return TOKEN_LINECOMMENT;
		}

		if (ch == '[') {
			pgn_buffer_line(pgn->buffer, buf + bufpos, outlen - bufpos);
			return TOKEN_TAG;
		}

		pgn_buffer_unget(pgn->buffer, ch);
		return TOKEN_TAG_END;
	}

	if (pgn->state == PARSE_GARBAGE) {
		if (pgn_buffer_eof(pgn->buffer)) return TOKEN_EOI;

		buf = outbuf;
		pgn_buffer_line(pgn->buffer, buf, outlen);

		x = buf;
		while (*x) {
			if (!isspace(*x)) break;
			x++;
		}

		if (*x == '[') return TOKEN_TAG;

		return TOKEN_LINECOMMENT;
	}

	return game_token(pgn->buffer, outbuf, outlen);
}


int pgn_open(pgn_t* pgn, const char* filename)
{
	fen_state_t fen;

	if (!pgn) return PGN_ERROR_BADPTR;

	pgn->buffer = pgn_buffer_new_file(filename);
	pgn->state = PARSE_GARBAGE;
	
	pgn->tree.root = pgnnode_alloc();

	pgn->tree.count = 0;
	pgn->tree.depth = 0;

	pgn->game.board = (board_t*)malloc(sizeof(board_t));

	/* initialize our ace board, we will use this to validate moves */
	memset(pgn->game.board, 0, sizeof(board_t));
	memset(pgn->game.board->pos.squares, INVALID_PIECE, 64 * sizeof(u8));

	pgn->game.undo.count = 0;

	/* initialize the starting position from FEN */
	fen_init(&fen);
	fen_use_ptr(&fen, pgn->game.board);
	fen_parse(&fen, FEN_OPENING, strlen(FEN_OPENING));
	fen_destroy(&fen);

	return PGN_SUCCESS;
}


void pgn_close(pgn_t* pgn)
{
	if (pgn) {
		pgn_buffer_free(pgn->buffer);
		pgn->state = PARSE_GARBAGE;

		free(pgn->tree.root);
		pgn->tree.count = 0;
		pgn->tree.depth = 0;

		free(pgn->game.board);
		pgn->game.undo.count = 0;
	}
}


int pgn_parse_moves(pgn_t* pgn, char* buffer, u32 buflen)
{
	pgn_token_t token;
	int rc = PGN_SUCCESS;

	if (!pgn) return PGN_ERROR_BADPTR;

	token = pgn_token(pgn, buffer, buflen);

	while (!is_result(token)) {
		switch (token) {
		case TOKEN_MOVE_PAWN:
		case TOKEN_MOVE_PROMOTE:
		case TOKEN_MOVE_PIECE:
		case TOKEN_MOVE_CASTLE:
		case TOKEN_MOVE_CASTLE_LONG:
		case TOKEN_MOVE_NULL:
			rc = pgn_parse_move(pgn, buffer, buflen, token);

			/* we can do some things here if we aren't set to strict castling */
			break;
		default: break;
		}

		token = pgn_token(pgn, buffer, buflen);
	}

	switch (token) {
	case TOKEN_RESULT_WHITE: pgn->game.result = WIN_WHITE; break;
	case TOKEN_RESULT_BLACK: pgn->game.result = WIN_BLACK; break;
	case TOKEN_RESULT_DRAW: pgn->game.result = DRAW; break;
	default: pgn->game.result = UNKNOWN_RESULT; break;
	}

	return rc;
}


int pgn_parse(pgn_t* pgn)
{
	char* buffer = (char*)malloc(MAX_TOKEN_SIZE);
	pgn_token_t token;
	int rc = PGN_ERROR_EOF;

	token = pgn_token(pgn, buffer, MAX_TOKEN_SIZE);

	while (token != TOKEN_EOI) {
		if (is_tag(token)) {
			if (pgn->state == PARSE_GARBAGE) {
				/* starting a new game */
				pgn->state = PARSE_HEADER;
			}

			/* TODO: parse the tag here */
		} else if (token == TOKEN_TAG_END) {
			pgn->state = PARSE_MOVES;

			rc = pgn_parse_moves(pgn, buffer, MAX_TOKEN_SIZE);
			break;
		} else if ((token == TOKEN_COMMENT) ||
				   (token == TOKEN_LINECOMMENT)) {
			/* do nothing */
		} else {
			/* invalid! */
			return PGN_ERROR_INVALID;
		}

		token = pgn_token(pgn, buffer, MAX_TOKEN_SIZE);
	}

	if (pgn->state == PARSE_HEADER) {
		return PGN_ERROR_GAME;
	}

	free(buffer);

	return rc;
}

#include "ace_fen.h"
#define PGN_FILE "F:\\Games\\The Opera Game.pgn"


static int check_for_mate(board_t* board)
{
	undolist_t ul;
	movelist_t ml;
	u16 i;

	ul.count = 0;

	if (check(board, board->side)) {
		generate_moves(board, &ml, &ml);

		for (i = 0; i < ml.count; i++) {
			if (do_move(board, &ul, ml.moves[i]) == TRUE) {
				undo_move(board, &ul);
				return FALSE;
			}
		}
	}

	return TRUE;
}

/**
 * Writes the move m on board board in algebraic notation into buffer. The
 * number of bytes written is returned.  The number of bytes returned will
 * never exceed buflen.
 */
static int write_move(board_t* board, move_t m, char *buffer, u16 buflen)
{
	u16 i, cnt = 0;
	movelist_t ml;
	move_t z;
	piece_type_t p, o;
	piece_t piece = INVALID_PIECE;
	u16 found = 0, ck = 0;
	char mc = '+';
	undolist_t ul;
	char amb = 0;

	if (!buffer) return 0;
	if (buflen < 2) return 0;
	if (!board) return 0;

	ul.count = 0;
	p = piece_type(board->pos.squares[move_from(m)]);

	generate_moves(board, &ml, &ml);

	for (i = 0; i < ml.count; i++) {
		z = ml.moves[i];
		o = piece_type(board->pos.squares[move_from(z)]);

		/* look for ambiguities (identical to sqaure, identical piece type) */
		if ((move_to(m) == move_to(z)) &&
			(move_from(m) != move_from(z)) &&
			(p == o)) {
			/* use file by default, unless they're equal, then use rank */
			if (file(move_from(m)) == file(move_from(z))) {
				amb = char_from_rank(rank(move_from(m)));
			} else {
				amb = char_from_file(file(move_from(m)));
			}
		}

		if (m == z) found = 1;
	}

	if (!found) return 0;

	if (do_move(board, &ul, m) == TRUE) {
		if (check(board, board->side)) {
			ck = 1;
			if (check_for_mate(board) == TRUE) {
				mc = '#';
			}
		}

		undo_move(board, &ul);
	}

	if (p == PAWN) {
		/* first check the buffer length */
		cnt = 2;
		if (is_capture(m)) cnt += 2;
		if (is_promotion(m)) cnt += 2;
		if (ck == 1) cnt++;
		if (buflen < cnt) return 0;

		cnt = 0;

		buffer[cnt++] = char_from_file(file(move_from(m)));

		if (is_capture(m)) {
			buffer[cnt++] = 'x';
			buffer[cnt++] = char_from_file(file(move_to(m)));
		}

		buffer[cnt++] = char_from_rank(rank(move_to(m)));

		if (is_promotion(m)) {
			buffer[cnt++] = '=';

			set_piece_color(piece, WHITE);
			set_piece_type(piece, (piece_type_t)(move_kind(m) & 0x03) + KNIGHT);

			buffer[cnt++] = char_from_piece(piece);
		}

		if (ck == 1) { buffer[cnt++] = mc; }
	} else if (move_kind(m) == KING_CASTLE) {
		cnt = 3;
		if (ck == 1) cnt++;
		if (buflen < cnt) return 0;

		cnt = 0;

		buffer[cnt++] = 'O';
		buffer[cnt++] = '-';
		buffer[cnt++] = 'O';

		if (ck == 1) { buffer[cnt++] = mc; }
	} else if (move_kind(m) == QUEEN_CASTLE) {
		cnt = 5;
		if (ck == 1) cnt++;
		if (buflen < cnt) return 0;

		cnt = 0;

		buffer[cnt++] = 'O';
		buffer[cnt++] = '-';
		buffer[cnt++] = 'O';
		buffer[cnt++] = '-';
		buffer[cnt++] = 'O';

		if (ck == 1) { buffer[cnt++] = mc; }
	} else {
		cnt = 3;
		if (is_capture(m)) cnt += 1;
		if (ck == 1) cnt++;
		if (amb != 0) cnt++;
		if (buflen < cnt) return 0;

		set_piece_color(piece, WHITE);
		set_piece_type(piece, p);
		cnt = 0;

		buffer[cnt++] = char_from_piece(piece);
		if (amb != 0) { buffer[cnt++] = amb; }

		if (is_capture(m)) {
			buffer[cnt++] = 'x';
		}

		buffer[cnt++] = char_from_file(file(move_to(m)));
		buffer[cnt++] = char_from_rank(rank(move_to(m)));

		if (ck == 1) { buffer[cnt++] = mc; }
	}

	buffer[cnt] = 0;

	return cnt;
}

typedef struct _linebuffer
{
	char *buf;
	u16 pos;		/* the current position in buf */
	u16 line;		/* the current line */
	u16 buflen;		/* the length of buf in bytes*/
	u16 linelen;	/* the length of a line in bytes */
} linebuffer_t;

linebuffer_t* create_line_buffer(u16 linelen, u16 size)
{
	linebuffer_t *rc = (linebuffer_t*)malloc(sizeof(linebuffer_t));

	if (rc) {
		rc->buf = (char*)malloc(size);
		rc->buflen = size;
		rc->linelen = linelen;
		rc->pos = 0;
		rc->line = 0;
	}

	return rc;
}

void destroy_line_buffer(linebuffer_t* buf)
{
	if (buf) {
		free(buf->buf);
	}

	free(buf);
}

void add_to_line(linebuffer_t* buffer, const char *sz, u16 szlen)
{
	char *s;
	u16 i;
	u16 offset, lineoff;

	if (szlen == 0) return;
	if ((!buffer) || (!sz)) return;
	if (buffer->pos >= (buffer->buflen - 1)) return;

	offset = buffer->pos + szlen;
	lineoff = (buffer->pos - (buffer->linelen * buffer->line)) + szlen;
	s = buffer->buf;

	if (lineoff >= (buffer->linelen - 1)) { 
		s[buffer->pos++] = '\n';
		buffer->line++;
		offset++;
	}

	if (offset < buffer->buflen) {
		for (i = 0; i < szlen; i++) {
			s[buffer->pos++] = sz[i];
		}
	}

	s[buffer->pos] = 0;
}

void pgntree_print(pgn_tree_t* tree, game_t* game)
{
	pgn_node_t *child = NULL, *parent;
	move_t m;
	u16 i, sum = 0;
	float *tdi;
	float* tdf;
	char buffer[128];
	table_t *table;

	if ((!tree) || (!tree->root)) return;

	parent = tree->root;

	for (i = 0; ((parent) && (i < game->undo.count)); i++) {
		m = game->undo.undo[i].move;
		child = parent->eldest;

		while (child) {
			if (child->sat.move == m) break;

			child = child->sibling;
		}

		parent = child;
	}


	if (!parent) {
		printf("no variations recorded\n");
	} else {
		child = parent->eldest;

		tdi = (float*)malloc(parent->sat.nchild * sizeof(float));
		tdf = (float*)malloc(parent->sat.nchild * sizeof(float));
		i = 0;

		table = table_create(parent->sat.nchild, 2);

		while (child) {
			write_move(game->board, child->sat.move, buffer, 128);

			tdi[i] = (float)child->sat.count;
			tdf[i] = ((float)child->sat.points / 10.0f) / (float)child->sat.count;
			sum += child->sat.count;

			table_set_key(table, i, buffer);
			
			i++;
			child = child->sibling;
		}

		if (sum == 0) { sum = 1; }
		for (i = 0; i < parent->sat.nchild; i++) { 
			tdi[i] /= (float)sum;
			tdi[i] *= 100.0f;
			tdf[i] *= 100.0f;
		}

		column_from_float(&table->columns[0], ALIGN_RIGHT, "%2.1f %%", tdi);
		column_from_float(&table->columns[1], ALIGN_RIGHT, "%2.1f %%", tdf);

		table_print(table);

		table_destroy(table);

		free(tdi);
		free(tdf);
	}
}

void print_game(undolist_t *ul)
{
	if (!ul) return;

	fen_state_t fen;
	undolist_t replay;
	u16 i, n;
	char buf[64], sp = ' ';
	move_t w, b;

	linebuffer_t *lines;

	/* re-initialize the board to the starting position */
	fen_init(&fen);
	fen_parse(&fen, FEN_OPENING, strlen(FEN_OPENING));

	replay.count = 0;

	lines = create_line_buffer(79, 4096);

	for (i = 0; i < ul->count; i += 2) {
		w = ul->undo[i].move;

		n = snprintf(buf, 64, "%d. ", (i / 2) + 1);
		add_to_line(lines, buf, n);

		/* print move */
		n = write_move(fen.board, w, buf, 64);
		add_to_line(lines, buf, n);
		add_to_line(lines, &sp, 1);

		/* execute the move */
		do_move(fen.board, &replay, w);

		if ((i + 1U) < ul->count) {
			b = ul->undo[i + 1].move;

			/* print move */
			n = write_move(fen.board, b, buf, 64);
			add_to_line(lines, buf, n);
			add_to_line(lines, &sp, 1);

			/* execute the move */
			do_move(fen.board, &replay, b);
		}
	}

	printf("%s", lines->buf);

	destroy_line_buffer(lines);
	fen_destroy(&fen);
}

void pgn_new_game(pgn_t* pgn)
{
	pgn->state = PARSE_GARBAGE;

	new_game(&pgn->game);
}

void pgn_test()
{
	pgn_t pgn;

	pgn_open(&pgn, PGN_FILE);

	while (pgn_parse(&pgn) == PGN_SUCCESS) {
		/* we can use our board for whatever */
//		print_game(&pgn.game.undo);
		printf(".");
		fflush(stdout);

		pgntree_add(&pgn.tree, &pgn.game);

		pgn_new_game(&pgn);
	}

	pgn_close(&pgn);
}
