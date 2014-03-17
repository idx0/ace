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
extern board_rank_t pawn_enpas_rank[2]; 
extern board_rank_t pawn_double_rank[2];
extern piece_type_t promoted_type[4];
extern int piece_material_values[6];
/* piece-square tables */
extern int pawn_pcsq[64];
extern int knight_pcsq[64];
extern int bishop_pcsq[64];
extern int rook_pcsq[64];
extern int queen_pcsq[64];
extern int king_pcsq[64];
extern int king_endgame_pcsq[64];
/* pawn isolation and passing tables */
extern int pawn_passed[2][64];
extern int pawn_isolated[64];
extern int pawn_score_isolated;
extern int pawn_score_passed[8];
extern int pawn_score_backward[8];
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

/**
 * Initializes the precomputed movelist arrays
 */
extern void init_movelists();
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
 * @return The number of moves generated
 */
extern u32 generate_moves(const board_t* board, movelist_t* ml);
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
 * Processes a move in algebraic notation from user input
 * @param board The board structure
 * @param sz The input string
 * @param len The length of the input string
 * @param s The color of the side this move should be checked for
 * @return The move, if parsed correctly or 0 if invalid
 */
extern move_t process_algebraic(board_t* board, const char *sz,
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
   