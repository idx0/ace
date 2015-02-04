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
#include <ctype.h>
#include <assert.h>

#include "ace_types.h"
#include "ace_fen.h"
#include "ace_global.h"
#include "ace_display.h"
#include "ace_pgn.h"
#include "ace_str.h"

/******************************************************************************/

cmd_string_t* cmdstring_create(u16 len)
{
    cmd_string_t* buffer = (cmd_string_t*)malloc(sizeof(cmd_string_t));

    if (buffer) {
        buffer->buffer = (char*)malloc(len);
        buffer->count = len;
        buffer->length = 0;
        buffer->pos = 0;

        buffer->buffer[0] = 0;
    }

    return buffer;
}


void cmdstring_destroy(cmd_string_t* buffer)
{
    if (buffer) {
        free (buffer->buffer);
    }

    free(buffer);
}


void cmdstring_add_to_buffer(cmd_string_t* buffer, char c)
{
    if (buffer) {
        if (buffer->length < buffer->count) {
            buffer->buffer[buffer->length++] = c;
        }

        buffer->buffer[buffer->length] = 0;
    }
}


void cmdstring_reset(cmd_string_t* buffer)
{
    if (buffer) {
        buffer->length = 0;
        buffer->pos = 0;
        buffer->buffer[0] = 0;
    }
}


int cmdstring_next_word(cmd_string_t* buffer, char* out, u16 outlen)
{
    u16 i = 0;
    u16 inq = 0, indq = 0;
    u16 esc = 0;
    char c;

    if (!buffer) return 0;
    if (buffer->pos >= buffer->length) return 0;
    if (buffer->pos >= buffer->count) return 0;

    /* move past spaces */
    while (isspace(buffer->buffer[buffer->pos])) {
        buffer->pos++;
    }

    /* copy chars */
    do {
        c = buffer->buffer[buffer->pos++];

        if (c == 0) {
            break;
        } else if (esc == 1) {
            esc = 0;
            switch (c) {
            case 'n': out[i++] = '\n'; break;
            case 't': out[i++] = '\t'; break;
            case 'r': out[i++] = '\r'; break;
            case 'b': out[i++] = '\b'; break;
            case 'f': out[i++] = '\f'; break;
            case 'v': out[i++] = '\v'; break;
            default:
                 out[i++] = c;
                 break;
            }
        } else if ((indq == 0) && (inq == 0) && (isspace(c))) {
            break;
        } else if (c == '\\') {
            esc = 1;
        } else if (c == '\'') {
            inq = !inq;
        } else if (c == '"') {
            indq = !indq;
        } else {
            out[i++] = c;
        }
    } while ((i < outlen) && (buffer->pos < buffer->length));

    out[i] = 0;

    return i;
}

/******************************************************************************/

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
    u8 src = INVALID_SQUARE;
    piece_t piece;

    assert(board);
    assert(ml);
    assert(is_valid_index(dest_sq));

    for (i = 0; i < ml->count; i++) {
        to = move_to(ml->moves[i]);
        from = move_from(ml->moves[i]);

        piece = board->pos.squares[from];

        /* is the dest square and piece type the same? */
        if ((to == dest_sq) && (piece_type(piece) == pt)) {
            src = from;
            break;
        }
    }

    return src;
}


int command_quit(cmd_string_t* cmdbuf)
{
    static const char cmd_quit[] = "quit";
    static const char cmd_exit[] = "exit";

    char out[MAX_CMD_LEN];
    u16 len = cmdstring_next_word(cmdbuf, out, MAX_CMD_LEN);

    if (len >= 4) {
        if (strncmp(out, cmd_quit, 4) == 0) {
            return TRUE;
        } else if (strncmp(out, cmd_exit, 4) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


int process_moves(board_t *board, undolist_t *ul, char *sz,
                  size_t len, side_color_t s)
{
    static const char delim[] = " ,.";
    char *ptr, *ctx = NULL;
    move_t m;
    int nmoves = 0;

    ptr = strtok2(sz, delim, &ctx);
    while (ptr) {
        m = process_algebraic(board, ptr, strlen(ptr), s);

        if (m) {
            do_move(board, ul, m);
            nmoves++;
        } else {
            fprintf(stderr, "ace: process moves stopped - invalid move `%s'\n", ptr);
        }

        ptr = strtok2(NULL, delim, &ctx);
    }

    return nmoves;
}


move_t process_algebraic(board_t* board, const char *sz,
						 size_t len, side_color_t s)
{
	move_t output;
	piece_type_t piece = PAWN;
	board_rank_t dest_rank = 0;
	board_file_t dest_file = 0;
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

    assert(board);
    assert(sz);
    assert(len);

    move_think = TBEGIN;

	/* loop through the string, trying to figure out what type of move we have */
    for (i = 0; i < len; i++) {
    	c = sz[i];

    	if (isfile(c)) {
			if (move_think == TBEGIN) {
				move_think = TPAWN;
			} else if (move_think == TPAWN) {
				/* bishop move */
				move_think = TQUIET;
				piece = process_piece_type(c);
    		} else if (dest_file != 0) {
                move_think = TERROR;
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
                dest_file = 0;
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
            move_think = TERROR;
    		fprintf(stderr, "ace: character `%c' invalid.\n", c);
    	}
    }

    /* ok, let's generate a list of valid moves and verify */
    generate_moves(board, &ml, &ml);

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


move_t process_long_notation(board_t* board, const char *sz,
                             size_t len, side_color_t s)
{
    piece_type_t piece = PAWN;
    board_file_t tof = 0, fromf = 0;
    board_rank_t tor = 0, fromr = 0;
    move_t output = 0;
    size_t i;
    char c;
    int valid = TRUE;
    movelist_t ml;
    undolist_t ul;

    /*
     * normal move [from file][from rank][to file][to rank]
     * null move 0000
     * pawn promotion [from file][from rank][to file][to rank][piece type]
     */
    enum {
        TBEGIN, TFROM, TTO, TNORMAL, TPROMO, TNULL, TERROR
    } move_think;

    assert(board);
    assert(sz);
    assert(len);

    move_think = TBEGIN;

    for (i = 0; i < len; i++) {
        c = sz[i];

        if (isfile(c)) {
            if (move_think == TBEGIN) {
                move_think = TFROM;
                fromf = (c - 'a');
            } else if (move_think == TFROM) {
                move_think = TTO;
                tof = (c - 'a');
            }
        } else if (isrank(c)) {
            if (move_think == TFROM) {
                fromr = (c - '1');
            } else if (move_think == TTO) {
                tor = (c - '1');
                move_think = TNORMAL;
            }
        } else if (iswhite(c) || isblack(c)) {
            if (move_think == TNORMAL) {
                move_think = TPROMO;
                piece = process_piece_type(c);
            }
        } else if (c == '0') {
            if (move_think == TBEGIN) {
                move_think = TNULL;
            }
        } else {
            move_think = TERROR;
            fprintf(stderr, "ace: character `%c' invalid.\n", c);
        }
    }

    /* niavely generate a move, don't worry about kind */
    switch (move_think) {
        case TNORMAL:
        case TPROMO:
            output = to_move(from_rank_file(fromr, fromf),
                             from_rank_file(tor, tof),
                             0);
            break;
        default: output = 0; break;
    }

    /* if we couldn't generate a move, return */
    if (!output) return 0;

    /* ok, let's generate a list of valid moves and verify */
    generate_moves(board, &ml, &ml);

    /* finally, check if the move we think we made is in our move list */
    ul.count = 0;
    for (i = 0; i < ml.count; i++) {
        /* found our move */
        if ((ml.moves[i] & 0x0fff) == output) {
            /* check promotion */
            if (move_think == TPROMO) {
                if ((is_promotion(ml.moves[i])) &&
                    (promoted_type[move_kind(ml.moves[i]) & 0x03] == piece))
                {
                    /* move valid */
                }
                else
                    valid = FALSE;
            }

            if ((valid) && (do_move(board, &ul, output))) {
                /* the move is ok, the king is not left in check */
                undo_move(board, &ul);

                return ml.moves[i];
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


/* OS independent input ready function as used by olithink, crafy, and others */
int input_ready()
{
#ifdef ACE_UNIX
    struct timeval tv;
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE h;
    DWORD dw;

    if (!init) {
        init = 1;
        h = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(h, &dw);

        if (!pipe) {
            SetConsoleMode(h, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(h);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(h, NULL, 0, NULL, &dw, NULL)) {
            return TRUE;
        }

        return dw;
    } else {
        GetNumberOfConsoleInputEvents(h, &dw);
        return (dw <= 1 ? FALSE : dw);
    }
#endif
}


void checkup(app_t *app)
{
    char buf[256];

    if (input_ready()) {
        fgets(buf, 256, stdin);
        printf("%s\n");
    }
}


static int command_done(cmd_string_t* cmdbuf)
{
    static const char cmd[] = "done";
    char out[MAX_CMD_LEN];

    u16 len = cmdstring_next_word(cmdbuf, out, MAX_CMD_LEN);

    if (len >= 4) {
        if (strncmp(out, cmd, 4) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


static int is_shorthand(const char *ptr, size_t len, char match)
{
    return ((len == 1) && (ptr[0] == match));
}


static int command_info(app_t *app, char **ctx)
{
    static const char delim[] = " \t\r\n";
    char *ptr;
    ptr = strtok2(NULL, delim, ctx);

    if (ptr) {
        if (strncmp(ptr, "hash", 4) == 0) {
            printf("hash table -----\n");
            printf("  record ptr=     %p\n", app->hash.record);
            printf("  entries=        %d\n", app->hash.entries);
            printf("  overwrites=     %d\n", app->hash.overwritten);
            printf("  hits=           %d\n", app->hash.hit);
            printf("  hits w/ cutoff= %d\n", app->hash.cut);
            printf("  size (mb)=      %d\n", (1 << app->hash.size) / (1024 * 1024));
            printf("  size (records)= %d\n", app->hash.exist);
            printf("  age=            %d\n", app->hash.generations);
        } else if (strncmp(ptr, "search", 6) == 0) {
            printf("search -----\n");
            printf("  depth=          %d\n", app->search.depth);
            printf("  movesleft=      %d\n", app->search.movesleft);
            printf("  nodes=          %lld\n", app->search.nodes);
            printf("  flags=          %c%c%c%c%c%c\n",
                (app->search.flags & SEARCH_DEPTH ? 'D' : '-'),
                (app->search.flags & SEARCH_TIMED ? 'T' : '-'),
                (app->search.flags & SEARCH_STOPPED ? 'S' : '-'),
                (app->search.flags & SEARCH_PONDER ? 'P' : '-'),
                (app->search.flags & SEARCH_INFINITE ? 'I' : '-'),
                (app->search.flags & SEARCH_NULL ? 'N' : '-'));
        }
    }

    return TRUE;
}


static void process_ls(app_t *app)
{
    u32 i, s, cnt = 0;
    movelist_t ml;
    undolist_t ul;
    char *sz;

    assert(app);
	assert(app->game.board);

	print_board(app->game.board);
    printf("\n");

    ul.count = 0;
	generate_moves(app->game.board, &ml, &ml);

    for (i = 0; i < ml.count; i++) {
		if (do_move(app->game.board, &ul, ml.moves[i])) {
            /* the move is ok, the king is not left in check */
			sz = str_algebraic(app->game.board->pos.squares[move_to(ml.moves[i])], ml.moves[i]);
            s = strlen(sz);

            if ((cnt + s) > 80) {
                printf("\n");
                cnt = 0;
            }

            cnt += s + 1;
            printf("%s ", sz);

			undo_move(app->game.board, &ul);
        }
    }

    printf("\n");
}

#ifdef ACE_LINUX
#define PGN_DB "pgn/eco.pgn"
#else
#define PGN_DB "F:\\Games\\Kasparov.pgn"
#endif

static int process_ace_command(app_t *app, char *sz, size_t len)
{
    static const char delim[] = " \t\r\n";
    move_t m;
    char *ptr, *ctx = NULL;
    size_t cmdlen;
    int tmp, rc;

    ptr = strtok2(sz, delim, &ctx);

    if (ptr) {
        cmdlen = strlen(ptr);

        if ((strncmp(ptr, "print", 5) == 0) ||
            is_shorthand(ptr, cmdlen, 'p')) {
            printf("\n");
			print_board(app->game.board);
        } else if (strncmp(ptr, "fen", 3) == 0) {
            app->mode = IFEN;
        } else if ((strncmp(ptr, "move", 4) == 0) ||
                   is_shorthand(ptr, cmdlen, 'm')) {
            app->mode = IMOVE;
        } else if (strncmp(ptr, "uci", 3) == 0) {
            app->mode = IUCI;
            init_uci();
        } else if (strncmp(ptr, "init", 4) == 0) {
            init_startpos(app);
        } else if (strncmp(ptr, "perft", 5) == 0) {
            perft_kiwipete();
            printf("-----\n");
            pertf_runtests();
        } else if (strncmp(ptr, "think", 5) == 0) {
            ptr = strtok2(NULL, delim, &ctx);
            if (ptr) {
                tmp = atoi(ptr);
                if ((tmp <= 0) || (tmp > SEARCH_MAXDEPTH - 1))
                    app->search.depth = 8;
                else
                    app->search.depth = tmp;
                think(app);
            }
        } else if (strncmp(ptr, "test", 4) == 0) {
			/* test(); */
        } else if (strncmp(ptr, "ls", 7) == 0) {
            process_ls(app);
        } else if (strncmp(ptr, "pgn", 3) == 0) {
			pgn_open(&app->pgn, PGN_DB);
			tmp = 0;

			while ((rc = pgn_parse(&app->pgn)) != PGN_ERROR_EOF) {
				switch (rc) {
				case PGN_ERROR_GAME:
					printf("\ninvalid game parameter [%d]\n", tmp);
					break;
				case PGN_ERROR_MOVE:
					printf("\nillegal move detected [%d]\n", tmp);
					break;
				case PGN_ERROR_BADPTR:
					printf("\ninvalid pointer [%d]\n", tmp);
					break;
				default:
					pgntree_add(&app->tree, &app->pgn.game);

					printf(".");
					fflush(stdout);
					break;
				}

				if (rc == PGN_ERROR_MOVE) {
					print_game(&app->pgn.game.undo);
					break;
				}

				pgn_new_game(&app->pgn);
				tmp++;
			}

//			pgn_close(&app->pgn);
		} else if (strncmp(ptr, "tree", 4) == 0) {
			pgntree_print(&app->tree, &app->game);
		} else if (strncmp(ptr, "info", 4) == 0) {
            return command_info(app, &ctx);
        } else if (strncmp(ptr, "help", 4) == 0) {
        } else {
            /* maybe this is a move */
			if ((m = process_algebraic(app->game.board, sz, len, app->game.board->side))) {
				do_move(app->game.board, &app->game.undo, m);
			} else if ((m = process_long_notation(app->game.board, sz, len, app->game.board->side))) {
				do_move(app->game.board, &app->game.undo, m);
            } else {
                return FALSE;
            }
        }
    }

    return TRUE;
}


int process_command(app_t *app, cmd_string_t* cmdbuf)
{
    fen_state_t fen;
    
    char *sz = cmdbuf->buffer;
    u16 len = cmdbuf->length;

    assert(app);

    /* check for quit first */
    if (command_quit(cmdbuf)) {
        app->quit = TRUE;
    /* check for 'done' which takes the user out of the current mode */
    } else if (command_done(cmdbuf)) {
        app->mode = IACE;
    } else {
        switch (app->mode)
        {
            case IMOVE:
				process_moves(app->game.board, &app->game.undo, sz, len, app->game.board->side);
                break;
            case IFEN:
                fen_init(&fen);
				fen_use_ptr(&fen, app->game.board);
                fen_parse(&fen, sz, len);
                fen_destroy(&fen);
                app->mode = IACE;
                break;
            case IACE:
                process_ace_command(app, sz, len);
                break;
            case IUCI:
                parse_uci(app, sz, len);
                break;
            default: break;
        }
    }

    return TRUE;
}
