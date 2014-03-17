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
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "ace_types.h"
#include "ace_fen.h"
#include "ace_global.h"

static piece_type_t process_piece_type(char c)
{
	piece_type_t p = PAWN;

	if ((c == 'r') || (c == 'R')) p = ROOK;
	else if ((c == 'q') || (c == 'Q')) p = QUEEN;
	else if ((c == 'k') || (c == 'K')) p = KING;
	else if ((c == 'b') || (c == 'B')) p = BISHOP;
	else if ((c == 'n') || (c == 'N')) p = KNIGHT;

	return p;
}


static u8 get_source_sq(const board_t* board, const movelist_t* ml,
                        const u32 dest_sq, const piece_type_t pt)
{
    int i;
    u32 to, from;
    u8 kind;
    u8 src = INVALID_SQUARE;
    piece_t piece;

    assert(board);
    assert(ml);
    assert(is_valid_index(dest_sq));

    for (i = 0; i < ml->count; i++) {
        to = move_to(ml->moves[i]);
        from = move_from(ml->moves[i]);
        kind = move_kind(ml->moves[i]);

        piece = board->pos.squares[from];

        /* is the dest square and piece type the same? */
        if ((to == dest_sq) && (piece_type(piece) == pt)) {
            src = from;
            break;
        }
    }

    return src;
}


int command_quit(const char *sz, size_t len)
{
    static const char q[] = "quit";

    if (len >= 4) {
        if (strncmp(sz, q, 4) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


int process_moves(board_t *board, undolist_t *ul, char *sz,
                  size_t len, side_color_t s)
{
    static const char delim[] = " ,.";
    char *ptr;
    move_t m;
    int nmoves = 0;

    ptr = strtok(sz, delim);
    while (ptr) {
        m = process_algebraic(board, ptr, strlen(ptr), s);

        if (m) {
            do_move(board, ul, m);
            nmoves++;
        } else {
            printf("process_moves stopped - invalid move `%s'\n", ptr);
        }

        ptr = strtok(NULL, delim);
    }

    return nmoves;
}


move_t process_algebraic(board_t* board, const char *sz,
						 size_t len, side_color_t s)
{
	move_t output;
	piece_type_t piece;
	board_rank_t dest_rank;
	board_file_t dest_file, src_file = 0;
	u8 kind, to, from;
	movelist_t ml;
	undolist_t ul;
	size_t i;
	char c;

	/*
     * pawn moves: [dest file][dest rank]
     * piece moves [piece][dest file][dest rank]
     * pawn captures [src file]x[dest file][dest rank]
     * piece captures [piece]x[dest file][dest rank]
     * pawn promotion [dest file][dest rank]=[piece]
     * pawn promotion capture [src file]x[dest file][dest rank]=[piece]
     * king castle O-O
     * queen castle O-O-O
	 */
    enum {
    	TBEGIN, TPAWN, TQUIET, TPAWNCAP, TCAP, TPROMO,
    	TPROMOCAP, TCASTLE, TKING, TQUEEN, TERROR
    } move_think;

    move_think = TBEGIN;

	/* loop through the string, trying to figure out what type of move we have */
    for (i = 0; i < len; i++) {
    	c = sz[i];

    	if (isfile(c)) {
    		if (move_think == TBEGIN) {
    			move_think = TPAWN;
    		}
    		dest_file = (c - 'a');
    	} else if (isrank(c)) {
    		dest_rank = (c - '1');
    	} else if (iswhite(c) || isblack(c)) {
    		if (move_think == TBEGIN) {
    			move_think = TQUIET;
    		}
    		piece = process_piece_type(c);
    	} else if (c == 'x') {
    		if (move_think == TQUIET) {
    			move_think = TCAP;
    		} else if (move_think == TPAWN) {
    			src_file = dest_file;
    			move_think = TPAWNCAP;
    		} else {
    			move_think = TERROR;
    		}
    	} else if (c == '=') {
    		/* if we have seen an 'x' already, we better think we are a pawn
    		   capture */
    		if (move_think == TPAWNCAP) {
    			move_think = TPROMOCAP;
    		} else if (TPAWN) {
    			move_think = TPROMO;
    		} else {
				move_think = TERROR;
    		}
    	} else if ((c == '+') || (c == '#')) {
    		/* these generally indicate check or checkmate, which I don't know 
    		   how to support yet */
    	} else if (c == 'O') {
    		if (move_think == TBEGIN) {
    			move_think = TCASTLE;
            } else if ((TKING) || (TQUEEN)) {
                /* noop */
    		} else {
    			move_think = TERROR;
    		}
    	} else if (c == '-') {
    		if (move_think == TCASTLE) {
    			move_think = TKING;
    		} else if (move_think == TKING) {
    			move_think = TQUEEN;
    		}
    	} else {
    		/* invalid */
    		printf("character `%c' invalid.\n", c);
    	}
    }

    /* ok, let's generate a list of valid moves and verify */
    generate_moves(board, &ml);

    if (move_think == TKING) {
        if (s == WHITE) {
            output = to_move(E1, G1, KING_CASTLE);
        } else {
            output = to_move(E8, G8, KING_CASTLE);
        }
    } else if (move_think == TQUEEN) {
        if (s == WHITE) {
            output = to_move(E1, C1, QUEEN_CASTLE);
        } else {
            output = to_move(E8, C8, QUEEN_CASTLE);
        }
    } else {
        to = from_rank_file(dest_rank, dest_file);

        if ((move_think == TPROMO) || (move_think == TPROMOCAP) ||
            (move_think == TPAWN) || (move_think == TPAWNCAP)) {
            from = get_source_sq(board, &ml, to, PAWN);
        } else {
            from = get_source_sq(board, &ml, to, piece);
        }

        output = 1;

        switch (move_think) {
            default: output = 0; break;
            case TPROMO:
                kind = KNIGHT_PROMO + (piece - KNIGHT);
                break;
            case TPROMOCAP:
                kind = KNIGHT_PROMO_CAP + (piece - KNIGHT);
                break;
            case TPAWN:
                if (abs(rank(to) - rank(from)) >= 2) {
                    /* double pawn move */
                    kind = DOUBLE_PAWN;
                } else {
                    kind = QUIET;
                }
                break;
            case TQUIET:
                kind = QUIET;
                break;
            case TPAWNCAP:
                if (to == board->enpas) {
                    kind = EP_CAPTURE;
                } else {
                    kind = CAPTURE;
                }
            case TCAP:
                kind = CAPTURE;
                break;
        }

        if (output) {
            output = to_move(from, to, kind);
        }
    }

    /* if we couldn't generate a move, return */
    if (!output) return 0;

    /* finally, check if the move we think we made is in our move list */
    ul.count = 0;
    for (i = 0; i < ml.count; i++) {
    	/* found our move */
    	if (ml.moves[i] == output) {
    		if (do_move(board, &ul, output)) {
    			/* the move is ok, the king is not left in check */
    			undo_move(board, &ul);

                return output;
    		} else {
    			ml.moves[i] = 0;
    		}
    	} else {
    		/* set the move to zero */
    		ml.moves[i] = 0;
    	}
    }

    /* if we get here, the move is illegal or invalid */
    return 0;
}
