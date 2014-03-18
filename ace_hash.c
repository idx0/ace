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

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_global.h"

void init_hash(hash_table_t *table, u16 mb)
{

}


void store_hash(hash_table_t *table, board_t *board, const move_t move,
				int score, int flags, int depth)
{
	u32 i;

	assert(table);
	assert(board);

	i = board->key & table->size;

	if (table->record[i].key != 0) {
		table->overwritten++;
	}

	table->entries++;

	table->record[i].move = move;
	table->record[i].key = board->key;
	table->record[i].flags = flags;
	table->record[i].score = score;
	table->record[i].depth = depth;
}


int probe_hash(hash_table_t *table, board_t *board, move_t *outmove, int *outscore,
			   int depth, int alpha, int beta)
{
	hash_record_t *rec = &table->record[board->key & table->size];

	assert(outmove);
	assert(outscore);

	if (rec->key == board->key) {
		*outmove = rec->move;

		if (rec->depth >= depth) {
			table->hit++;

			*outscore = rec->score;

			if (rec->flags & HASH_ALPHA) {
				if (rec->score <= alpha) {
					*outscore = alpha;
				}
				return TRUE;
			} else if (rec->flags & HASH_BETA) {
				if (rec->score >= beta) {
					*outscore = beta;
				}
				return TRUE;
			} else if (rec->flags & HASH_EXACT) {
				return TRUE;
			}
		}
	}

	return FALSE;
}
