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

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_fen.h"

/* This file defines global, precomputed bitboards and other values.  We are
   not concerned with memory here */

extern u32 bitscan_8bit[256];
/* this bitboard has a 1 set if the board color for that square is "dark" */
extern u64 board_colors;
/* These bitboard arrays hold all valid move positions for the respective
   piece at the board position given by the array index */
u64 knight_movelist[64];
u64 bishop_movelist[64];
u64 rook_movelist[64];
u64 queen_movelist[64];
u64 king_movelist[64];
/* Pawns are different because their move rules are dependent on their color
   and whether they are capturing a piece. */
u64 pawn_movelist[2][64];
u64 pawn_capturelist[2][64];
/* This bitboard has a bit set if the relative index in pawn_movelist should
   have an en passant square set */
u64 pawn_enpas[2];
/* This bitboard is used to check for pawn promotion */
u64 pawn_promotion[2];
/* This bitboard has represents pawn start double move positions */
u64 pawn_enpas_move[2][8];
/* This bitboard represents the castling moves for the given castling permissions */
u64 castle_movelist[16];
/* This bitboard represents the spaces that must be unoccupied in order for
   castling to be valid */
u64 castle_unocc[16];
/* This bitboard represents the ray the king will travel on to make a given
   castling move */
u64 castle_ray[16];
u8 castle_side[16];
/* These are piece arrays representing the to and from squares a rook will move
   given a new king location after a castling move */
u8 castle_rook_to[64];
u8 castle_rook_from[64];
/* This array defines the castle permissions for a given square */
extern u8 castle_permission[64];
/* The rank of the pawn en passant sqaure for each color */
extern board_rank_t pawn_enpas_rank[2]; 
/* The rank of the pawn double move square for each color */
extern board_rank_t pawn_double_rank[2];
/* An array of promoted types */
extern piece_type_t promoted_type[4];
/* piece material values */
extern int piece_material_values[6];
/* mate value */
#define MATE 32768
/* piece-square tables */
extern int pawn_pcsq[64];
extern int knight_pcsq[64];
extern int bishop_pcsq[64];
extern int rook_pcsq[64];
extern int queen_pcsq[64];
extern int king_pcsq[64];
extern int king_endgame_pcsq[64];

/* pawn position tables for passed pawns (or also backward pawns) */
extern u64 pawn_passed[2][64];
/* pawn position tables for isolated pawns */
extern u64 pawn_isolated[8];
/* position flip table for board squares */
extern u32 flipsq[2][64];
/* rank masks */
extern u64 bboard_ranks[8];
/* file masks */
extern u64 bboard_files[8];

extern int pawn_score_isolated;
extern int pawn_score_passed[8];
extern int pawn_score_backward[8];
/* LvvMva array */
extern int move_score_mvvlva[6][6];
extern int move_score_special[16];
/* these are pawn diagonal tables used to mask with the pawn position in order
   to find pawn chains */
extern u64 pawn_chain_ne[9];
extern u64 pawn_chain_nw[9];

#ifdef CALCULATE_RAYS
/* These bitboards are the "rays" used to move rooks, bishops, and queens.
   When generating the actual moves these pieces can make, we draw a ray in
   each direction from the piece and cutoff the ray if it intersects another
   piece on the board */
u64 ray_topleft[64];
u64 ray_top[64];
u64 ray_topright[64];
u64 ray_right[64];
u64 ray_bottomright[64];
u64 ray_bottom[64];
u64 ray_bottomleft[64];
u64 ray_left[64];
/* These variables are used for simplicity */
enum ray_dir { TOPLEFT = 0, NORTHWEST = 0,
			   TOP = 1, NORTH = 1,
			   TOPRIGHT = 2, NORTHEAST = 2,
			   RIGHT = 3, EAST = 3,
			   BOTTOMRIGHT = 4, SOUTHEAST = 4,
			   BOTTOM = 5, SOUTH = 5,
			   BOTTOMLEFT = 6, SOUTHWEST = 6,
			   LEFT = 7, WEST = 7 } ray_dir_t;
u64* ray_list[8];
#define is_positive_ray(rd) ((rd) <= RIGHT)
#endif

/* bitboards representing the outer squares and the inner 6 squares */
#define outer_squares 0xff818181818181ff
#define inner_squares 0x007e7e7e7e7e7e00

extern void test();

/**
 * Initializes the precomputed movelist arrays
 */
extern void init_movelists();
extern void init_app(app_t *app);
extern void init_startpos(app_t *app);
extern void destroy_app(app_t *app);
/* Returns a bitboard representing all the squares attacked by side s.  This 
 * does not include the pawn who made an en passant move.
 * @param board The board structure
 * @param s The side of the attacker
 * @return Bitboard of all attacked squares
 */
extern u64 attacking(const board_t* board, const side_color_t s);
/**
 * Checks if the given square is under attack by the given side.  This function
 * is indentical to what attacking() returns except it will return true if the
 * given square contains a pawn which just made a double move and whose en passant
 * square is under attack.
 * @param board The board structure
 * @param sq The square that will be checked if under attack
 * @param s The side of the attacker
 * @return TRUE if sq is attacked by side s, FALSE otherwise
 */
extern int is_attacked(const board_t* board, const u32 sq, const side_color_t s);
/**
 * Generates all valid moves for the given board configuration
 * @param board The board structure
 * @param ml The movelist into which the generated moves will be stored
 * @param cl The movelist into which capture moves will be generated (this value
 *           can be set to ml if capture moves do not need to be generated
 *           separately).
 * @return The number of moves generated
 */
extern u32 generate_moves(board_t* board, movelist_t* ml, movelist_t *cl);
/**
 * Removes the piece at sq.
 * @param board The board structure
 * @param sq The index position of the piece to be removed
 * @return The removed piece
 */
extern piece_t remove_piece(board_t* board, const u32 sq);
/**
 * Adds the given piece on the given square
 * @param board The board structure
 * @param sq The index position of the piece to be added
 * @param piece The piece to add
 */
extern void add_piece(board_t* board, const u32 sq, const piece_t piece);
/**
 * Adds the given piece on the given square
 * @param board The board structure
 * @param from The index position of the piece
 * @param to The index position where the piece will be moved
 * @return piece The moved piece
 */
extern piece_t move_piece(board_t* board, const u32 from, const u32 to);
/**
 * Performs the move given by move
 * @param board The board structure
 * @param ul The undolist history for this match
 * @param move The move to be executed
 * @return TRUE if the move is legal, FALSE otherwise
 */
extern int do_move(board_t* board, undolist_t* ul, const move_t move);
/**
 * Undos the last move in ul
 * @param board The board structure
 * @param ul The undolist history for this match
 */
extern void undo_move(board_t* board, undolist_t* ul);

/**
 * These two functions perform and revert NULL moves
 * @param board The board structure
 * @param ul The undolist history for this match
 */
extern void do_null_move(board_t* board, undolist_t* ul);
extern void undo_null_move(board_t* board, undolist_t* ul);

/**
 * Generates cache data for the given side
 * @param board The board structure
 * @param s The side color to generate cache data for
 */
extern void generate_cache(board_t* board, const side_color_t s);

/**
 * Checks if the king for side color s is in check
 * @return TRUE if check, FALSE otherwise
 */
extern int check(const board_t* board, const side_color_t s);

/**
 * Performs a perft test to the given depth
 * @param board The board structure
 * @param ul The undolist history for this match
 * @param depth The depth of the given test
 * @return The number of nodes visited
 */
extern u64 perft(board_t* board, undolist_t* ul, int depth);
extern void perft_kiwipete();
extern void pertf_runtests();

/**
 * Evaluates the given board position relative to the side to move
 * @param board The board structure
 */
extern int evaluate(board_t* board);

/**
 * Perform iterative deepening search on the given board position
 * @param app A pointer to the  application handle
 */
extern void think(app_t *app);

/**
 * The "checkup" function which looks for input on stdin
 * @param app A pointer to the  application handle
 */
extern void checkup(app_t *app);

/**
 * Processes a move in algebraic notation from user input
 * @param board The board structure
 * @param sz The input string
 * @param len The length of the input string
 * @param s The color of the side this move should be checked for
 * @return The move, if parsed correctly or 0 if invalid
 */
extern move_t process_algebraic(board_t* board, const char *sz,
                                size_t len, side_color_t s);
/**
 * Processes a move in long algebraic notation from user input
 * @param board The board structure
 * @param sz The input string
 * @param len The length of the input string
 * @param s The color of the side this move should be checked for
 * @return The move, if parsed correctly or 0 if invalid
 */
extern move_t process_long_notation(board_t* board, const char *sz,
                                    size_t len, side_color_t s);

extern int process_moves(board_t *board, undolist_t *ul, char *sz,
                         size_t len, side_color_t s);
/**
 * Returns TRUE if the user has entered the 'quit' command
 * @param sz The input string
 * @param len The length of the input string
 * @return TRUE if quit, FALSE otherwise
 */
extern int command_quit(const char *sz, size_t len);

/**
 * Processes a command from the given string
 * @param quit A pointer to the application handle
 * @param sz The input string
 * @param len The length of the input string
 * @return TRUE if the command was processed succesfully, FALSE otherwise
 */
extern int process_command(app_t *app, char *sz, size_t len);

/**
 * OS inpendent function which returns true if input is ready to be read from
 * stdin
 * @return TRUE if input is ready, FALSE otherwise
 */
extern int input_ready();

/**
 * Initializes the hash table to the number of mb given
 * @param table A pointer to the hash table structure
 * @param mb The size of the table in MB
 */
extern void init_hash(hash_table_t *table, u16 mb);

/**
 *
 * @param table A pointer to the hash table structure
 */
extern void store_hash(hash_table_t *table, board_t *board, const move_t move,
                       int score, int flags, int depth);

/**
 *
 * @param table A pointer to the hash table structure
 */
extern int probe_hash(hash_table_t *table, board_t *board, move_t *outmove,
                      int *outscore, int depth, int alpha, int beta);

/**
 * Probes the hash table for a stored move at the given key
 * @param table A pointer to the hash table structure
 * @param boardkey The hash to probe
 * @return The move associated with the given board key
 */
extern move_t probe_hash_move(hash_table_t *table, u64 boardkey);

/**
 * Initializes the UCI procotol mode
 */
extern void init_uci();

/**
 *
 */
extern int parse_uci(app_t *app, char *sz, size_t len);
