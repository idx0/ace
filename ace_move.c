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

static void add_quiet_move(const board_t *board, const u64 bboard,
						   const u32 from, movelist_t *ml)
{
	u32 to;
	u64 tmp = bboard;
	piece_t p;

	while (tmp) {
		to = ACE_LSB64(tmp);
		assert(ml->count < MAX_MOVES);
		ml->moves[ml->count] = to_move(from, to, QUIET);

		/* check killers */
		if (board->killers[0][board->ply] == ml->moves[ml->count])
			ml->scores[ml->count] = 9100;
		else if (board->killers[1][board->ply] == ml->moves[ml->count])
			ml->scores[ml->count] = 9000;
		/* otherwise, use the value from the history */
		else {
			p = board->pos.squares[from];
			ml->scores[ml->count] = 
				board->history[piece_color(p)][piece_type(p)][to] & 0x1fff;
		}

		ml->count++;
		tmp ^= (1ULL << to);
	}
}


static void add_capture_move(const board_t *board, const u64 bboard,
							 const u32 from, movelist_t *ml)
{
	u32 to;
	u64 tmp = bboard;

	while (tmp) {
		to = ACE_LSB64(tmp);

		assert(ml->count < MAX_MOVES);	

		ml->moves[ml->count] = to_move(from, to, CAPTURE);
		ml->scores[ml->count] =
			move_score_mvvlva[piece_type(board->pos.squares[from])]
							 [piece_type(board->pos.squares[to])];
		ml->count++;

		tmp ^= (1ULL << to);
	}
}


static void add_special_move(const board_t *board, const u64 bboard,
							 const u32 from, movelist_t *ml, u8 flag)
{
	u32 to;
	u64 tmp = bboard;

	while (tmp) {
		to = ACE_LSB64(tmp);
		assert(ml->count < MAX_MOVES);	
		ml->moves[ml->count] = to_move(from, to, flag);
		ml->scores[ml->count] = move_score_special[flag & 0x0f];
		ml->count++;
		tmp ^= (1ULL << to);
	}
}


int check(const board_t* board, const side_color_t s)
{
	side_color_t oc;

	assert(board);
	assert(is_valid_index(board->pos.king_sq[s]));

	oc = (~s & 0x01);

	return is_attacked(board, board->pos.king_sq[s], oc);
}


/* calling this function also updates cache variables to the extent it can 
   reasonably do so */
u64 attacking(const board_t* board, const side_color_t s)
{
	u64 pieces, occ, friendly, ret = 0ULL;
	u32 sq;
	side_color_t oc = (~s & 0x01);

	assert(board);
	assert(is_valid_index(board->pos.king_sq[s]));

	occ = (board->pos.occ[WHITE] | board->pos.occ[BLACK]);
	friendly = ~board->pos.occ[s];

	/* pawns */
	pieces = board->pos.piece[s][PAWN];
	while (pieces) {
		sq = ACE_LSB64(pieces);
		ret |= pawn_capturelist[s][sq];
		/* we'll need to generate pawn moves as well, but we can do that in
		   another function */
		board->pos.cache.mobility[sq] |= pawn_capturelist[s][sq] & friendly;
		pieces ^= (1ULL << sq);
	}

	/* kinghts */
	pieces = board->pos.piece[s][KNIGHT];
	while (pieces) {
		sq = ACE_LSB64(pieces);
		ret |= knight_movelist[sq];
		board->pos.cache.mobility[sq] |= knight_movelist[sq] & friendly;
		pieces ^= (1ULL << sq);
	}

	/* bishops & queens */
	pieces = (board->pos.piece[s][BISHOP] | board->pos.piece[s][QUEEN]);
	while (pieces) {
		sq = ACE_LSB64(pieces);
		ret |= magic_bishop(sq, occ);
		board->pos.cache.mobility[sq] |= magic_bishop(sq, occ) & friendly;
		pieces ^= (1ULL << sq);
	}

	/* rooks & queens */
	pieces = (board->pos.piece[s][ROOK] | board->pos.piece[s][QUEEN]);
	while (pieces) {
		sq = ACE_LSB64(pieces);
		ret |= magic_rook(sq, occ);
		board->pos.cache.mobility[sq] |= magic_rook(sq, occ) & friendly;
		pieces ^= (1ULL << sq);
	}

	/* kings */
	ret |= king_movelist[board->pos.king_sq[s]];
	board->pos.cache.mobility[sq] |= king_movelist[board->pos.king_sq[s]] & friendly;

	/* at this point ret is equal to any position on the board that a piece of
	   side s can reach */

	/* positions we can attack are equal to every position we can get to that
	   has an opponent occupying it */
	board->pos.cache.attack[s] = ret & board->pos.occ[s];
	/* likewise, positions we can defend are equal to every position we can get
	   to that has a friendly on it */
	board->pos.cache.defend[s] = ret & board->pos.occ[oc];

	/* now, we can AND either of these bitboard with a position to see if we can
	   attack it or defend it with the given color */

	return ret & friendly;
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
		(board->pos.piece[oc][PAWN] & sqbb) &&
		(pawn_enpas_move[oc][file(board->enpas)] == sqbb)) {
		/* set the enpas square pawn as attacked */
		board->pos.cache.attack[s] |= sqbb;
		return TRUE;
	}

	return FALSE;
}


static void generate_knight_moves(const board_t* board, movelist_t* ml, 
								  movelist_t* cl, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;
	pieces = board->pos.piece[s][KNIGHT];

	/* knight moves */
	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		/* add the move from our precomputed move list */
		add_quiet_move(board, (knight_movelist[i] & ~occ), i, ml);
		add_capture_move(board, (knight_movelist[i] & board->pos.occ[oc]), i, cl);

		/* xor out the bit we just checked */
		pieces ^= (1ULL << i);
	}
}


static void generate_bishop_queen_moves(const board_t* board, movelist_t* ml, 
										movelist_t* cl, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces, move;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	pieces = board->pos.piece[s][BISHOP] | board->pos.piece[s][QUEEN];

	/* bishop and queen moves */
	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		/* add the move as generated from the magic tables */
		move = magic_bishop(i, occ);
		add_quiet_move(board, (move & ~occ), i, ml);
		add_capture_move(board, (move & board->pos.occ[oc]), i, cl);

		/* xor out the bit we just checked */
		pieces ^= (1ULL << i);
	}
}


static void generate_rook_queen_moves(const board_t* board, movelist_t* ml, 
									  movelist_t* cl, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces, move;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	pieces = board->pos.piece[s][ROOK] | board->pos.piece[s][QUEEN];

	/* rook and queen moves */
	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		/* add the move as generated from the magic tables */
		move = magic_rook(i, occ);
		add_quiet_move(board, (move & ~occ), i, ml);
		add_capture_move(board, (move & board->pos.occ[oc]), i, cl);

		/* xor out the bit we just checked */
		pieces ^= (1ULL << i);
	}
}


static void generate_pawn_moves(const board_t* board, movelist_t* ml, 
								movelist_t* cl, const u64 occ)
{
	side_color_t s, oc;
	u32 i;
	u64 pieces, move, capture;

	assert(board);
	assert(ml);

	oc = (~board->side & 0x01);
	s = board->side;

	pieces = board->pos.piece[s][PAWN];

	while (pieces) {
		/* we should do LSB for white and MSB for black,
		   but for now it really doesn't matter */
		i = ACE_LSB64(pieces);

		move = pawn_movelist[s][i] & ~occ;
		capture = (pawn_capturelist[s][i] & board->pos.occ[oc]);

		/* If move is non-zero, it means a pawn is able to move.  If it is moving
		   to the en passant square, add another move if we can. */
		if (move & pawn_enpas[s]) {
			/* en passant moves are available */
			add_special_move(board, (pawn_enpas_move[s][file(i)] & ~occ), i, ml, DOUBLE_PAWN);
		}

		if ((move & pawn_promotion[s]) || (capture & pawn_promotion[s])) {
			add_special_move(board, move, i, ml, KNIGHT_PROMO);
			add_special_move(board, move, i, ml, BISHOP_PROMO);
			add_special_move(board, move, i, ml, ROOK_PROMO);
			add_special_move(board, move, i, ml, QUEEN_PROMO);
			add_special_move(board, capture, i, cl, KNIGHT_PROMO_CAP);
			add_special_move(board, capture, i, cl, BISHOP_PROMO_CAP);
			add_special_move(board, capture, i, cl, ROOK_PROMO_CAP);
			add_special_move(board, capture, i, cl, QUEEN_PROMO_CAP);
		} else {
			add_quiet_move(board, move, i, ml);
			add_capture_move(board, capture, i, cl);
		}

		if (is_valid_index(board->enpas)) {
			move = (1ULL << board->enpas);
			add_special_move(board, (pawn_capturelist[s][i] & move), i, cl, EP_CAPTURE);
		}

		pieces ^= (1ULL << i);
	}
}


static void generate_king_moves(const board_t* board, movelist_t* ml, 
								movelist_t* cl, const u64 occ)
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
	i = board->pos.king_sq[s];

	assert(is_valid_index(board->pos.king_sq[s]));

	/* get all attacked positions */
	opp = attacking(board, oc);

	/* get all legal moves of this king */
	moves = king_movelist[i];

	/* AND in ~opp to ensure the king cannot move to an attacked position */
	add_quiet_move(board, (moves & ~occ & ~opp), i, ml);
	add_capture_move(board, (moves & board->pos.occ[oc] & ~opp), i, cl);

	/* generate castling moves */
	castle_bits = (board->castle & (3 << (2 * s)));
	check = (board->pos.piece[s][KING] & opp);

	moves = 0ULL;
	while (castle_bits) {
		j = bitscan_8bit[castle_bits];
		c = (1 << j);

		/* ray attacked OR path occupied */
		bad = (castle_ray[c] & opp) | (castle_unocc[c] & occ) | check;
		if (!bad) add_special_move(board, castle_movelist[c], i, ml, castle_side[c]);

		castle_bits ^= c;
	}
}


u32 generate_moves(const board_t* board, movelist_t* ml, movelist_t *cl)
{
	u64 occ;

	assert(board);
	assert(ml);
	assert(cl);

	ml->count = 0;
	cl->count = 0;
	occ = board->pos.occ[WHITE] | board->pos.occ[BLACK];

	memset(&board->pos.cache, 0, sizeof(eval_cache_t));

	generate_rook_queen_moves(board, ml, cl, occ);
	generate_bishop_queen_moves(board, ml, cl, occ);
	generate_knight_moves(board, ml, cl, occ);
	generate_pawn_moves(board, ml, cl, occ);
	generate_king_moves(board, ml, cl, occ);

	board->pos.cache.valid = TRUE;

	return ((ml == cl) ? ml->count : ml->count + cl->count);
}


piece_t remove_piece(board_t* board, const u32 sq)
{
	u64 sqbb = (1ULL << sq);
	piece_t piece;
	piece_type_t type;
	piece_color_t color;

	assert(is_valid_index(sq));
	assert(board);
	assert(piece_valid(board->pos.squares[sq]));

	piece = board->pos.squares[sq];
	color = piece_color(piece);
	type = piece_type(piece);

	assert(sqbb & board->pos.piece[color][type]);

	/* hash out the piece */
	board->key ^= hash_piece[color][type][sq];

	/* clear the piece */
	board->pos.squares[sq] = INVALID_PIECE;
	board->pos.piece[color][type] ^= sqbb;
	board->pos.occ[color] ^= sqbb;

	board->pos.material[color] -= piece_material_values[type];
	board->pos.npieces--;

	switch (type) {
		case PAWN:
			board->pos.pawns ^= sqbb;
			break;
		case KNIGHT:
		case BISHOP:
			board->pos.minors[color]--;
			break;
		case ROOK:
		case QUEEN:
			board->pos.majors[color]--;
			break;
		case KING:	/* this better not happen! */
			board->pos.king_sq[color] = INVALID_SQUARE;
			break;
	}

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
	assert(!(board->pos.piece[color][type] & sqbb));

	/* assert that sq is empty */
	assert(!((board->pos.occ[WHITE] | board->pos.occ[BLACK]) & sqbb));

	/* hash in the piece */
	board->key ^= hash_piece[color][type][sq];

	/* add the piece */
	board->pos.squares[sq] = piece;
	board->pos.piece[color][type] ^= sqbb;
	board->pos.occ[color] ^= sqbb;

	board->pos.material[color] += piece_material_values[type];
	board->pos.npieces++;

	switch (type) {
		case PAWN:
			board->pos.pawns ^= sqbb;
			break;
		case KNIGHT:
		case BISHOP:
			board->pos.minors[color]++;
			break;
		case ROOK:
		case QUEEN:
			board->pos.majors[color]++;
			break;
		case KING:
			board->pos.king_sq[color] = sq;
			break;
	}
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
	assert(piece_valid(board->pos.squares[from]));

	piece = board->pos.squares[from];
	color = piece_color(piece);
	type = piece_type(piece);

	assert(frombb & board->pos.piece[color][type]);
	assert(!((board->pos.occ[WHITE] | board->pos.occ[BLACK]) & tobb));

	/* hash out from, in to */
	board->key ^= hash_piece[color][type][from];
	board->key ^= hash_piece[color][type][to];

	/* clear from piece */
	board->pos.squares[from] = INVALID_PIECE;
	board->pos.piece[color][type] ^= frombb;
	board->pos.occ[color] ^= frombb;

	/* add to piece */
	board->pos.squares[to] = piece;
	board->pos.piece[color][type] ^= tobb;
	board->pos.occ[color] ^= tobb;

	switch (type) {
		case PAWN:
			board->pos.pawns ^= frombb;
			board->pos.pawns ^= tobb;
			break;
		case KING:
			board->pos.king_sq[color] = to;
			break;
		default: break;
	}

	return piece;
}


int do_move(board_t* board, undolist_t* ul, const move_t move)
{
	u32 from = move_from(move);
	u32 to = move_to(move);
	u8 kind = move_kind(move);
	side_color_t oc, s;
	piece_t promo = INVALID_PIECE;

	assert(is_valid_index(from));
	assert(is_valid_index(to));
	assert(ul);
	assert(board);
	assert(piece_valid(board->pos.squares[from]));

	s = board->side;
	oc = (~board->side & 0x01);

	/* before we start modifying, store the key in history */
	ul->undo[ul->count].key = board->key;

	/* move castling rook */
	if ((kind == KING_CASTLE) || (kind == QUEEN_CASTLE)) {
		move_piece(board, castle_rook_from[to], castle_rook_to[to]);
	}

	/* hash out the en passant square */
	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];
	/* hash out the castling permission */
	board->key ^= hash_castle[board->castle];

	ul->undo[ul->count].move = move;
	ul->undo[ul->count].enpas = board->enpas;
	ul->undo[ul->count].castle = board->castle;
	ul->undo[ul->count].capture = INVALID_PIECE;

	board->castle &= (castle_permission[from] & castle_permission[to]);
	board->enpas = INVALID_SQUARE;

	/* hash back in the new castling permission */
	board->key ^= hash_castle[board->castle];

	/* remove en passant capture */
	if (kind == EP_CAPTURE) {
		board->half = 0;
		remove_piece(board, from_rank_file(pawn_double_rank[oc], file(to)));
	}

	/* captures */
	if ((kind == CAPTURE) || (kind >= 0x0c)) {
		board->half = 0;
		ul->undo[ul->count].capture = board->pos.squares[to];
		remove_piece(board, to);
	}

	/* en passant */
	if (kind == DOUBLE_PAWN) {
		board->half = 0;
		board->enpas = from_rank_file(pawn_enpas_rank[s], file(to));
		board->key ^= hash_enpas[file(board->enpas)];
	}

	if (piece_type(board->pos.squares[from]) == PAWN) board->half = 0;

	/* finally store the ply, which may now be 0 */
	ul->undo[ul->count].ply = board->half;

	board->half++;
	board->ply++;
	ul->count++;

	/* finally, move the piece */
	move_piece(board, from, to);

	/* promotions */
	if (is_promotion(move)) {
		set_piece_type(promo, promoted_type[move_kind(move) & 0x03]);
		set_piece_color(promo, s);

		remove_piece(board, to);
		add_piece(board, to, promo);
	}

	if (board->side == BLACK) board->moves++;

	/* flip the side */
	board->side = oc;
	/* if we were white, now we are black, so we hash the side in, otherwise
	   we are hashing the side out */
	board->key ^= hash_side;

	/* are we in check? */
	if (check(board, s)) {
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
	side_color_t s, oc;

	assert(board);
	assert(ul);

	ul->count--;
	board->ply--;

	from = move_from(ul->undo[ul->count].move);
	to = move_to(ul->undo[ul->count].move);
	kind = move_kind(ul->undo[ul->count].move);

	assert(is_valid_index(from));
	assert(is_valid_index(to));

	s = board->side;
	oc = (~board->side & 0x01);

	if (oc == BLACK) board->moves--;

	/* hash out en passant and board castle permissions */
	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];
	board->key ^= hash_castle[board->castle];

	board->castle = ul->undo[ul->count].castle;
	board->enpas = ul->undo[ul->count].enpas;
	board->half = ul->undo[ul->count].ply;

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


void do_null_move(board_t* board, undolist_t* ul)
{
	side_color_t oc;

	assert(board);
	assert(ul);

	oc = (~board->side & 0x01);

	ul->undo[ul->count].key = board->key;

	ul->undo[ul->count].move = 0;
	ul->undo[ul->count].ply = board->half;
	ul->undo[ul->count].enpas = board->enpas;
	ul->undo[ul->count].castle = board->castle;
	ul->undo[ul->count].capture = INVALID_PIECE;

	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];
	board->enpas = INVALID_SQUARE;

	ul->count++;
	board->ply++;
	board->moves++;

	/* flip the side */
	board->side = oc;
	/* if we were white, now we are black, so we hash the side in, otherwise
	   we are hashing the side out */
	board->key ^= hash_side;
}


void undo_null_move(board_t* board, undolist_t* ul)
{
	side_color_t oc;

	assert(board);
	assert(ul);

	oc = (~board->side & 0x01);

	ul->count--;
	board->ply--;
	board->moves--;

	board->castle = ul->undo[ul->count].castle;
	board->enpas = ul->undo[ul->count].enpas;
	board->half = ul->undo[ul->count].ply;

	if (is_valid_index(board->enpas)) board->key ^= hash_enpas[file(board->enpas)];

	/* flip the side */
	board->side = oc;
	/* if we were white, now we are black, so we hash the side in, otherwise
	   we are hashing the side out */
	board->key ^= hash_side;
}
