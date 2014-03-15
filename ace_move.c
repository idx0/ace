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

#include "ace_types.h"
#include "ace_magic.h"
#include "ace_global.h"
#include "ace_zobrist.h"

static void add_quiet_move(const u64 bboard, const u32 from, movelist_t *ml)
{
	u32 to;
	u64 tmp = bboard;

	while (tmp) {
		to = ACE_LSB64(tmp);
		assert(ml->count < MAX_MOVES);
		ml->moves[ml->count++] = to_move(from, to, QUIET);
		tmp ^= (1ULL << to);
	}
}


static void add_capture_move(const u64 bboard, const u32 from, movelist_t *ml)
{
	u32 to;
	u64 tmp = bboard;

	while (tmp) {
		to = ACE_LSB64(tmp);
		assert(ml->count < MAX_MOVES);	
		ml->moves[ml->count++] = to_move(from, to, CAPTURE);
		tmp ^= (1ULL << to);
	}
}


static void add_special_move(const u64 bboard, const u32 from, movelist_t *ml, u8 flag)
{
	u32 to;
	u64 tmp = bboard;

	while (tmp) {
		to = ACE_LSB64(tmp);
		assert(ml->count < MAX_MOVES);	
		ml->moves[ml->count++] = to_move(from, to, flag);
		tmp ^= (1ULL << to);
	}
}



u64 attacking(const board_t* board, const side_color_t s)
{
	side_color_t oc;
	u64 pieces, occ, friendly, ret = 0ULL;
	u32 i;

	assert(board);

	oc = (~s & 0x01);
	occ = (board->occ[WHITE] | board->occ[BLACK]);
	friendly = ~board->occ[s];

	/* pawns */
	pieces = board->piece[s][PAWN];
	while (pieces) {
		i = ACE_LSB64(pieces);
		ret |= pawn_capturelist[s][i] & friendly;
		pieces ^= (1ULL << i);
	}

	/* kinghts */
	pieces = board->piece[s][KNIGHT];
	while (pieces) {
		i = ACE_LSB64(pieces);
		ret |= knight_movelist[i] & friendly;
		pieces ^= (1ULL << i);
	}

	/* bishops & queens */
	pieces = (board->piece[s][BISHOP] | board->piece[s][QUEEN]);
	while (pieces) {
		i = ACE_LSB64(pieces);
		ret |= magic_bishop(i, occ) & friendly;
		pieces ^= (1ULL << i);
	}

	/* rooks & queens */
	pieces = (board->piece[s][ROOK] | board->piece[s][QUEEN]);
	while (pieces) {
		i = ACE_LSB64(pieces);
		ret |= magic_rook(i, occ) & friendly;
		pieces ^= (1ULL << i);
	}

	/* kings */
	ret |= king_movelist[ACE_LSB64(board->piece[s][KING])] & friendly;

	return ret;
}


int is_attacked(const board_t* board, const u32 sq, const side_color_t s)
{
	side_color_t oc;
	u64 sqbb = (1ULL << sq);
	u64 att;

	assert(board);

	oc = (~s & 0x01);

	att = attacking(board, s);

	if (att & sqbb) return TRUE;

	/* if the enpas square is set, and the enpas square is under attack,
	   and an enemy pawn is in the right spot to be captured */
	if ((is_valid_index(board->enpas)) && 
		(att & (1ULL << board->enpas)) &&
		(board->piece[oc][PAWN] & sqbb) &&
		(pawn_enpas_move[oc][file(board->enpas)] == sqbb)) {
		return TRUE;
	}

	return FALSE;
}


static void generate_knight_moves(const board_t* board, movelist_t* ml, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;
	pieces = board->piece[s][KNIGHT];

	/* knight moves */
	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		/* add the move from our precomputed move list */
		add_quiet_move((knight_movelist[i] & ~occ), i, ml);
		add_capture_move((knight_movelist[i] & board->occ[oc]), i, ml);

		/* xor out the bit we just checked */
		pieces ^= (1ULL << i);
	}
}


static void generate_bishop_queen_moves(const board_t* board, movelist_t* ml, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces, move;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	pieces = board->piece[s][BISHOP] | board->piece[s][QUEEN];

	/* bishop and queen moves */
	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		/* add the move as generated from the magic tables */
		move = magic_bishop(i, occ);
		add_quiet_move((move & ~occ), i, ml);
		add_capture_move((move & board->occ[oc]), i, ml);

		/* xor out the bit we just checked */
		pieces ^= (1ULL << i);
	}
}


static void generate_rook_queen_moves(const board_t* board, movelist_t* ml, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces, move;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	pieces = board->piece[s][ROOK] | board->piece[s][QUEEN];

	/* rook and queen moves */
	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		/* add the move as generated from the magic tables */
		move = magic_rook(i, occ);
		add_quiet_move((move & ~occ), i, ml);
		add_capture_move((move & board->occ[oc]), i, ml);

		/* xor out the bit we just checked */
		pieces ^= (1ULL << i);
	}
}


static void generate_pawn_moves(const board_t* board, movelist_t* ml, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces, move, capture;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	pieces = board->piece[s][PAWN];

	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		move = pawn_movelist[s][i] & ~occ;
		capture = (pawn_capturelist[s][i] & board->occ[oc]);

		/* If move is non-zero, it means a pawn is able to move.  If it is moving
		   to the en passant square, add another move if we can. */
		if (move & pawn_enpas[s]) {
			/* en passant moves are available */
			add_special_move((pawn_enpas_move[s][file(i)] & ~occ), i, ml, DOUBLE_PAWN);
		}

		if ((move & pawn_promotion[s]) || (capture & pawn_promotion[s])) {
			add_special_move(move, i, ml, KNIGHT_PROMO);
			add_special_move(move, i, ml, BISHOP_PROMO);
			add_special_move(move, i, ml, ROOK_PROMO);
			add_special_move(move, i, ml, QUEEN_PROMO);
			add_special_move(capture, i, ml, KNIGHT_PROMO_CAP);
			add_special_move(capture, i, ml, BISHOP_PROMO_CAP);
			add_special_move(capture, i, ml, ROOK_PROMO_CAP);
			add_special_move(capture, i, ml, QUEEN_PROMO_CAP);
		} else {
			add_quiet_move(move, i, ml);
			add_capture_move(capture, i, ml);
		}

		if (is_valid_index(board->enpas)) {
			move = (1ULL << board->enpas);
			add_special_move((pawn_capturelist[s][i] & move), i, ml, EP_CAPTURE);
		}

		pieces ^= (1ULL << i);
	}
}


static void generate_king_moves(const board_t* board, movelist_t* ml, const u64 occ)
{
	side_color_t s, oc;
	u32 i, j;
	u64 moves, opp, bad;
	u64 check;
	u8 castle_bits, c;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	i = ACE_LSB64(board->piece[s][KING]);

	/* get all attacked positions */
	opp = attacking(board, oc);

	/* get all legal moves of this king */
	moves = king_movelist[i];

	/* AND in ~opp to ensure the king cannot move to an attacked position */
	add_quiet_move((moves & ~occ & ~opp), i, ml);
	add_capture_move((moves & board->occ[oc] & ~opp), i, ml);

	/* generate castling moves */
	castle_bits = (board->castle & (3 << (2 * s)));
	check = (board->piece[s][KING] & opp);

	moves = 0ULL;
	while (castle_bits) {
		j = bitscan_8bit[castle_bits];
		c = (1 << j);

		/* ray attacked OR path occupied */
		bad = (castle_ray[c] & opp) | (castle_unocc[c] & occ) | check;
		if (!bad) add_special_move(castle_movelist[c], i, ml, castle_side[c]);

		castle_bits ^= c;
	}
}


u32 generate_moves(const board_t* board, movelist_t* ml)
{
	u64 occ;

	assert(board);
	assert(ml);

	ml->count = 0;
	occ = board->occ[WHITE] | board->occ[BLACK];

	generate_rook_queen_moves(board, ml, occ);
	generate_bishop_queen_moves(board, ml, occ);
	generate_knight_moves(board, ml, occ);
	generate_pawn_moves(board, ml, occ);
	generate_king_moves(board, ml, occ);

	return ml->count;
}


piece_t remove_piece(board_t* board, const u32 sq)
{
	u64 sqbb = (1ULL << sq);
	piece_t piece;
	piece_type_t type;
	piece_color_t color;

	assert(is_valid_index(sq));
	assert(board);
	assert(piece_valid(board->squares[sq]));

	piece = board->squares[sq];
	color = piece_color(piece);
	type = piece_type(piece);

	assert(sqbb & board->piece[color][type]);

	/* hash out the piece */
	board->key ^= hash_piece[color][type][sq];

	/* clear the piece */
	board->squares[sq] = INVALID_PIECE;
	board->piece[color][type] ^= sqbb;
	board->occ[color] ^= sqbb;

	return piece;
}


void add_piece(board_t* board, const u32 sq, const piece_t piece)
{
	u64 sqbb = (1ULL << sq);
	piece_type_t type;
	piece_color_t color;

	assert(is_valid_index(sq));
	assert(board);
	assert(piece_valid(piece));

	color = piece_color(piece);
	type = piece_type(piece);

	/* assert that sq does not already contain that piece */
	assert(!(board->piece[color][type] & sqbb));

	/* assert that sq is empty */
	assert(!((board->occ[WHITE] | board->occ[BLACK]) & sqbb));

	/* hash in the piece */
	board->key ^= hash_piece[color][type][sq];

	/* add the piece */
	board->squares[sq] = piece;
	board->piece[color][type] ^= sqbb;
	board->occ[color] ^= sqbb;
}


piece_t move_piece(board_t* board, const u32 from, const u32 to)
{
	u64 tobb = (1ULL << to);
	u64 frombb = (1ULL << from);
	piece_type_t type;
	piece_color_t color;
	piece_t piece;

	assert(is_valid_index(to));
	assert(is_valid_index(from));
	assert(board);
	assert(piece_valid(board->squares[from]));

	piece = board->squares[from];
	color = piece_color(piece);
	type = piece_type(piece);

	assert(frombb & board->piece[color][type]);
	assert(!((board->occ[WHITE] | board->occ[BLACK]) & tobb));

	/* hash out from, in to */
	board->key ^= hash_piece[color][type][from];
	board->key ^= hash_piece[color][type][to];

	/* clear from piece */
	board->squares[from] = INVALID_PIECE;
	board->piece[color][type] ^= frombb;
	board->occ[color] ^= frombb;

	/* add to piece */
	board->squares[to] = piece;
	board->piece[color][type] ^= tobb;
	board->occ[color] ^= tobb;

	return piece;
}


int do_move(board_t* board, undolist_t* ul, const move_t move)
{
	u32 from = move_from(move);
	u32 to = move_to(move);
	u8 kind = move_kind(move);
	u64 tobb = (1ULL << to);
	u64 frombb = (1ULL << from);
	side_color_t oc, s;
	piece_t promo;
	u64 opp;

	assert(is_valid_index(from));
	assert(is_valid_index(to));
	assert(ul);
	assert(board);
	assert(piece_valid(board->squares[from]));

	s = board->side;
	oc = (~board->side & 0x01);

	/* before we start modifying, store the key in history */
	ul->undo[ul->count].key = board->key;

	/* remove en passant capture */
	if (kind == EP_CAPTURE) {
		remove_piece(board, from_rank_file(pawn_double_rank[oc], file(to)));
	}

	/* move castling rook */
	if ((kind == KING_CASTLE) || (kind == QUEEN_CASTLE)) {
		move_piece(board, castle_rook_from[to], castle_rook_to[to]);
	}

	/* hash out the en passant square */
	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];
	/* hash out the castling permission */
	board->key ^= hash_castle[board->castle];

	ul->undo[ul->count].move = move;
	ul->undo[ul->count].plies = board->plies;
	ul->undo[ul->count].enpas = board->enpas;
	ul->undo[ul->count].castle = board->castle;
	ul->undo[ul->count].capture = INVALID_PIECE;

	board->castle &= (castle_permission[from] & castle_permission[to]);
	board->enpas = INVALID_SQUARE;

	/* hash back in the new castling permission */
	board->key ^= hash_castle[board->castle];

	board->plies++;

	/* captures */
	if ((kind == CAPTURE) || (kind >= 0x0c)) {
		board->plies = 0;
		ul->undo[ul->count].capture = board->squares[to];
		remove_piece(board, to);
	}

	ul->count++;
	board->moves++;
	board->search_ply++;

	/* en passant */
	if (kind == DOUBLE_PAWN) {
		board->plies = 0;
		board->enpas = from_rank_file(pawn_enpas_rank[s], file(to));
		board->key ^= hash_enpas[file(board->enpas)];
	}

	/* finally, move the piece */
	move_piece(board, from, to);

	/* promotions */
	if (is_promotion(move)) {
		set_piece_type(promo, promoted_type[move_kind(move) & 0x03]);
		set_piece_color(promo, s);

		remove_piece(board, to);
		add_piece(board, to, promo);
	}

	/* flip the side */
	board->side = oc;
	/* if we were white, now we are black, so we hash the side in, otherwise
	   we are hashing the side out */
	board->key ^= hash_side;

	/* are we in check? */
	opp = attacking(board, oc);
	if (board->piece[s][KING] & opp) {
		undo_move(board, ul);
		return FALSE;
	}

	return TRUE;
}


void undo_move(board_t* board, undolist_t* ul)
{
	static const piece_t sPAWNS[2] = { PAWN, (PAWN | 0x08) };

	u32 from, to;
	u8 kind;
	u64 tobb, frombb;
	side_color_t s, oc;

	assert(board);
	assert(ul);

	ul->count--;
	board->search_ply--;
	board->moves--;

	from = move_from(ul->undo[ul->count].move);
	to = move_to(ul->undo[ul->count].move);
	kind = move_kind(ul->undo[ul->count].move);

	assert(is_valid_index(from));
	assert(is_valid_index(to));

	frombb = (1ULL << from);
	tobb = (1ULL << to);
	s = board->side;
	oc = (~board->side & 0x01);

	/* hash out en passant and board castle permissions */
	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];
	board->key ^= hash_castle[board->castle];

	board->castle = ul->undo[ul->count].castle;
	board->enpas = ul->undo[ul->count].enpas;
	board->plies = ul->undo[ul->count].plies;

	/* hash in en passant and board castle permissions */
	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];
	board->key ^= hash_castle[board->castle];

	/* flip the side */
	board->side = oc;
	/* if we were white, now we are black, so we hash the side in, otherwise
	   we are hashing the side out */
	board->key ^= hash_side;

	/* add en passant capture */
	if (kind == EP_CAPTURE) {
		add_piece(board, from_rank_file(pawn_double_rank[s], file(to)), sPAWNS[s]);
	}

	/* revert castling rook */
	if ((kind == KING_CASTLE) || (kind == QUEEN_CASTLE)) {
		move_piece(board, castle_rook_to[to], castle_rook_from[to]);
	}

	/* finally, move the piece back */
	move_piece(board, to, from);

	/* revert capture */
	if ((kind == CAPTURE) || (kind >= 0x0c)) {
		assert(piece_valid(ul->undo[ul->count].capture));
		add_piece(board, to, ul->undo[ul->count].capture);
	}

	/* promotions */
	if (is_promotion(ul->undo[ul->count].move)) {
		remove_piece(board, from);
		add_piece(board, from, sPAWNS[oc]);
	}
}
