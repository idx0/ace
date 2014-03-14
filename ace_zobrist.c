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

#include "ace_zobrist.h"

void init_zobrist(const u32 seed)
{
	int i, c;

	rkiss_seed(seed);

	/* pieces */
	for (c = 0; c < 2; c++) {
		for (i = 0; i < 64; i++) {
			hash_pawns[c][i] = rkiss_rand();
			hash_knights[c][i] = rkiss_rand();
			hash_rooks[c][i] = rkiss_rand();
			hash_bishops[c][i] = rkiss_rand();
			hash_queens[c][i] = rkiss_rand();
			hash_kings[c][i] = rkiss_rand();
		}
	}

	hash_side = rkiss_rand();

	for (i = 0; i < 16; i++) {
		hash_castle[i] = rkiss_rand();
	}

	for (i = 0; i < 8; i++) {
		hash_enpas[i] = rkiss_rand();
	}
}
