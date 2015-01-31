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

#include "ace_thread.h"
#include "ace_types.h"

#define MAX_THREADS	64

typedef struct worker {
	/* the thread structure for this worker */
	thread_t thread;
	/* a numerical id to identify this thread in the pool index */
	u8 id;
	/* this mutex is used during node inspection */
	mutex_t node_mutex;
	/* the last node (parent) visited by this worker */
	node_t parent;
	/* the current node of this worker */
	node_t node;
	/* a pointer to the board for this worker */
	board_t *board;
	/* a boolean set to TRUE if this worker is idle */
	int idle;
} worker_t;

#define JOIN_WORKERS	1	/* once we get a reasonable depth, this flag will
							   indicate that we should join the workers */
#define SYNC_WORKER		2   /* this flag will be set if there are workers that
							   need to be synced */

typedef struct pool {
	/* the pool of workers */
	worker_t pool[MAX_THREADS];
	/* the actual number of workers */
	u8 nworkers;
	/* a mutex to lock when checking/setting flags for this pool */
	mutex_t flag_mutex;
	/* a list of flags for this pool */
	u32 flags;
} pool_t;


extern int check_flag(pool_t *pool, const u32 flag);
