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
#include <assert.h>

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_global.h"

static int repetitions(const app_t *app)
{
	int i;

	for (i = app->ul.count - app->board->half; i < app->ul.count; i++) {
		if (app->board->key == app->ul.undo[i].key) return TRUE;
	}

	return FALSE;
}


static void next_move(int num, movelist_t *ml)
{
	move_t m;
	int s, i;

	int best = 0;
	int found = num;

	for (i = num; i < ml->count; i++) {
		if (ml->scores[i] > best) {
			best = ml->scores[i];
			found  = i;
		}
	}

	if (found != num) {
		m = ml->moves[num];
		s = ml->scores[num];

		ml->moves[num] = ml->moves[found];
		ml->scores[num] = ml->scores[found];
		ml->moves[found] = m;
		ml->scores[found] = s;
	}
}


int quiescent(app_t *app, int alpha, int beta)
{
	return 0;
}


int alpha_beta(app_t *app, int alpha, int beta, int depth)
{
	int made = 0, palpha = alpha;
	int i, score = -MATE, highest = -MATE;
	int checked;
	movelist_t ml;
	move_t best, cutoff = 0;
	piece_t p;

	assert(app);

	/* recursive base */
	if (depth == 0) {
		app->search.nodes++;
		return evaluate(app->board);
	}

	app->search.nodes++;

	/* draws */
	if (repetitions(app) || (app->board->half >= 100)) {
		return 0;
	}

	/* max depth */
	if (app->board->ply > (SEARCH_MAXDEPTH - 1)) {
		return evaluate(app->board);
	}

	/* if we are checked, simply bump the depth by 1 */
	checked = check(app->board, app->board->side);
	if (checked) {
		depth++;
	}

	/* probe our table */
	if (probe_hash(&app->hash, app->board, &cutoff, &score, alpha, beta, depth) == TRUE) {
		app->hash.cut++;
		return score;
	}

	if (cutoff != 0) {
		for (i = 0; i < ml.count; i++) {
			if (ml.moves[i] == cutoff) {
				ml.scores[i] == 20000;
				break;
			}
		}
	}

	/* generate moves */
	generate_moves(app->board, &ml);

	/* search negamax */
	for (i = 0; i < ml.count; i++) {
		/* get the next move ordered */
		next_move(i, &ml);

		if (!(do_move(app->board, &app->ul, ml.moves[i])))
			continue;

		score = -alpha_beta(app, -beta, -alpha, depth - 1);

		made++;
		undo_move(app->board, &app->ul);

		/* score whatever is best so far */
		if (score > highest) {
			best = ml.moves[i];
			highest = score;

			/* update alpha */
			if (score > alpha) {
				if (score >= beta) {

					/* non-captures causing beta cutoffs (killers) */
					if (!is_capture(ml.moves[i])) {
						app->board->killers[1][app->board->ply] =
							app->board->killers[0][app->board->ply];
						app->board->killers[0][app->board->ply] = ml.moves[i];
					}

					/* store this beta in our transposition table */
					store_hash(&app->hash, app->board, best, beta, HASH_BETA, depth);

					return beta;
				}

				/* update alpha */
				alpha = score;

				/* update our history */
				if (!is_capture(ml.moves[i])) {
					p = app->board->pos.squares[move_from(best)];
					app->board->history[piece_color(p)][piece_type(p)][move_to(best)] += depth;
				}
			}
		}
	}

	/* check for checkmate or stalemate */
	if (!made) {
		if (check(app->board, app->board->side)) {
			return -MATE + app->board->ply;
		} else {
			return 0;
		}
	}

	if (alpha != palpha) {
		/* store this as an exact, since we beat alpha */
		store_hash(&app->hash, app->board, best, highest, HASH_EXACT, depth);
	} else {
		/* store the current alpha */
		store_hash(&app->hash, app->board, best, alpha, HASH_ALPHA, depth);
	}

	return alpha;
}


/**
 * The search is controlled by the think() function.  This function initializes
 * all variables needed for search, sets up thread pools, and then performs a
 * search starting at the root node.  A root node search is fundamentally
 * different from searches at other depths because we know for sure that we
 * should not perform a qsearch or a null-move search.
 */
void think(app_t *app)
{
	int i, val;

	assert(app);
	assert(app->board);

	/* reset our node count board->ply */
	app->board->ply = 0;
	app->search.nodes = 0;

	/* clear the stop bit */
	app->search.flags &= ~SEARCH_STOPPED;

	/* clear search heuristics */
	memset(&app->board->killers, 0, 2 * SEARCH_MAXDEPTH * sizeof(int));
	memset(&app->board->history, 0, 2 * 6 * 64 * sizeof(int));

	/* set the start time */
	get_current_tick(&app->search.start);

	/* iterative deepening */
	for (i = 1; i < app->search.depth; i++) {
		val = alpha_beta(app, -MATE, MATE, i);
	}
}
