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
#include <string.h>

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_global.h"


static void clear_hash(hash_table_t *table)
{
	u32 i, size = 0;

	assert(table);

	size = (1 << table->size) / sizeof(hash_record_t);

	for (i = 0; i < size; i++) {
		memset(&table->record[i], 0, sizeof(hash_record_t));
	}
}


void init_hash(hash_table_t *table, u16 mb)
{
	u32 size = 0;

	u8 bits = ((mb & 0x00ff) ? bitscan_8bit[mb & 0x00ff] :
							   bitscan_8bit[(mb >> 8) & 0x00ff] + 8);

	assert(table);

	table->entries = 0;
	table->overwritten = 0;
	table->hit = 0;
	table->cut = 0;
	table->size = 20 + bits;

	if (table->size > 31) { table->size = 31; }

	size = (1 << table->size);
	table->exist = size / sizeof(hash_record_t);

	/* try to align our table, whose record length is 16 bytes, on a 16 byte
	   boundary */
	table->record = (hash_record_t *)xmalloc(size, 16);

	assert(table->record);

	clear_hash(table);
}


void store_hash(hash_table_t *table, board_t *board, const move_t move,
				int score, int flags, int depth)
{
	u32 i, mask;

	assert(table);
	assert(board);

	mask = table->exist - 1;
	i = board->key & mask;

	/* If we are adding a hash that is within SEARCH_MAXDEPTH of MATE, then we
	   need to simply store MATE.  This is so that when we probe the table, we
	   can convert from MATE to MATE - (the current ply) invariant of the ply
	   this entry was stored at.  Plus we already know this ply (since it is
	   our depth value) */
	if ((MATE - abs(score)) <= SEARCH_MAXDEPTH) score = (score < 0 ? -1 : 1) * MATE;

	/* check if this entry is at a depth >= the depth already stored */
	/*if (depth >= table->record[i].depth) */ {
		if (table->record[i].key != 0) {
			table->overwritten++;
		} else {
			table->entries++;
		}

		table->record[i].move = move;
		table->record[i].key = board->key;
		table->record[i].flags = flags;
		table->record[i].score = score;
		table->record[i].depth = (u16)depth;
		table->record[i].age = 1;
	}
}


int probe_hash(hash_table_t *table, board_t *board, move_t *outmove, int *outscore,
			   int depth, int alpha, int beta)
{
	u32 mask = table->exist - 1;
	hash_record_t *rec = &table->record[board->key & mask];

	assert(outmove);
	assert(outscore);

	if (rec->key == board->key) {
		*outmove = rec->move;

		if (rec->depth >= depth) {
			table->hit++;

			*outscore = rec->score;

			/* if the recorded score is MATE, return MATE +- ply */
			if (abs(rec->score) == MATE) {
				*outscore -= get_sign(rec->score, 16) * board->ply;
			}

			/* check for our return */
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


move_t probe_hash_move(hash_table_t *table, u64 boardkey)
{
	u32 mask = table->exist - 1;
	hash_record_t *rec = &table->record[boardkey & mask];

	if (rec->key == boardkey) {
		return rec->move;
	}

	return 0;
}
