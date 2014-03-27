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

#include "ace_types.h"

extern void print_bboard(const u64 bb);
extern void print_bboard2(const u64 bb, const u64 bb2);
extern void print_board(const board_t* b);
/**
 * Prints a string representation of the move.  This function is NOT thread safe.
 */
extern void print_algebraic(const piece_t piece, const move_t move);
/**
 * Gets a string representation of the move.  This function is NOT thread safe.
 * @return The string representation of the given move in algebraic notation
 */
extern char* str_algebraic(const piece_t piece, const move_t move);
