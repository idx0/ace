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
	u32 i;

	assert(table);

	for (i = 0; i < table->exist; i++) {
		memset(&table->record[i].h[0], 0, sizeof(hash_record_t));
		memset(&table->record[i].h[1], 0, sizeof(hash_record_t));
		memset(&table->record[i].h[2], 0, sizeof(hash_record_t));
		memset(&table->record[i].h[3], 0, sizeof(hash_record_t));
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
	table->generations = 0;

	if (table->size > 31) { table->size = 31; }

	size = (1 << table->size);
	table->exist = size >> 7;

	/* try to align our table, whose record length is 16 bytes, on a 16 byte
	   boundary */
	table->record = (hash_cluster_t *)xmalloc(size, 64);

	assert(table->record);

	clear_hash(table);
}


void store_hash(hash_table_t *table, board_t *board, const move_t move,
				int score, int flags, int depth)
{
	u32 i, mask;
	move_t m = move;
	hash_record_t *rec, *add;

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

	rec = add = table->record[i].h;

	/* loop through the records in this cluster and look for an empty sport or
	   collision */
	for (i = 0; i < 4; i++, rec++) {
		/* is this an empty record */
		if (rec->key == 0) {
			/* new entry */
			table->entries++;
			add = rec;
			break;
		/* is this a record we can overwrite */
		} else if (rec->key == board->key) {
			/* overwrite old entry, save move */
			table->overwritten++;
			m = (move_t)rec->move;
			add = rec;
			break;
		/* is this a seed record (if so keep only 1) */
		} else if ((rec->flags == HASH_SEED) && (flags == HASH_SEED)) {
			add = rec;
			break;
		}

		/* maybe this guy is old, try to replace him */
		if (rec->age != (table->generations & 0xff))
			add = rec;
	}


	/* check if this entry is at a depth >= the depth already stored */
	if ((depth >= add->depth) || (flags == HASH_EXACT))
	{
#ifdef DONT_BEAT_EXACT
		if ((add->flags == HASH_EXACT) && (flags != HASH_EXACT)) return;
#endif

		add->move = m;
		add->key = board->key;
		add->flags = flags;
		add->score = (s16)score;
		add->depth = (u16)depth;
		add->age = (u8)(table->generations & 0xff);
	}
}


int probe_hash(hash_table_t *table, board_t *board, move_t *outmove, int *outscore,
			   int depth, int alpha, int beta)
{
	int i;
	u32 mask = table->exist - 1;
	hash_cluster_t *rec = &table->record[board->key & mask];

	assert(outmove);
	assert(outscore);

	for (i = 0; i < 4; i++) {
		if (rec->h[i].key == board->key) {
			*outmove = (move_t)rec->h[i].move;

			if (rec->h[i].depth >= depth) {
				table->hit++;

				*outscore = (s16)rec->h[i].score;

				/* if the recorded score is MATE, return MATE +- ply */
				if (abs(*outscore) == MATE) {
					*outscore -= get_sign(rec->h[i].score, 16) * board->ply;
				}

				/* check for our return */
				if (rec->h[i].flags & HASH_ALPHA) {
					if (*outscore <= alpha) {
						*outscore = alpha;
						return TRUE;
					}
				} else if (rec->h[i].flags & HASH_BETA) {
					if (*outscore >= beta) {
						*outscore = beta;
						return TRUE;
					}
				} else if ((rec->h[i].flags & HASH_EXACT) ||
						   (rec->h[i].flags & HASH_SEED)) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


move_t probe_hash_move(hash_table_t *table, u64 boardkey)
{
	int i;
	u32 mask = table->exist - 1;
	hash_cluster_t *rec = &table->record[boardkey & mask];

	for (i = 0; i < 4; i++) {
		if (rec->h[i].key == boardkey) {
			return (move_t)rec->h[i].move;
		}
	}

	return 0;
}
