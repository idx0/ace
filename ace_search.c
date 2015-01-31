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
#include <stdio.h>

#include "ace_intrin.h"
#include "ace_types.h"
#include "ace_global.h"
#include "ace_display.h"


static int repetitions(const app_t *app)
{
	unsigned int i;

	for (i = app->game.undo.count - app->game.board->half; i < app->game.undo.count; i++) {
		if (app->game.board->key == app->game.undo.undo[i].key) return TRUE;
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


static void init_node(node_t *node)
{
	node->flags = 0;
	node->ml.count = 0;
	node->cl.count = 0;
	node->best = 0;
	node->made = 0;
}


int quiescent(app_t *app, cnodeptr_t parent, int alpha, int beta)
{
	int i, score = -MATE;
	node_t node;
	int delta = 200;

	init_node(&node);

	assert(app);

	app->search.nodes++;

	/* max depth */
	if (app->game.board->ply > (SEARCH_MAXDEPTH - 1)) {
		return evaluate(app->game.board);
	}

	/* draws */
	if (repetitions(app) || (app->game.board->half >= 100)) {
		return 0;
	}

	score = evaluate(app->game.board);

	if (score >= beta) return beta;
	/* delta pruning based on a material value of delta (this should be disabled
	   in the endgame) */
	/* The idea here is that, even if our score can improve alpha, it doesn't
	   improve it by a significant amount, so don't bother searching these nodes */
	if (score < (alpha - delta)) return alpha;
	if (score > alpha) alpha = score;


	/* generate moves (with separate captures) */
	generate_moves(app->game.board, &node.ml, &node.cl);

	for (i = 0; i < node.cl.count; i++) {
		/* get the next move ordered */
		next_move(i, &node.cl);

		if (!(do_move(app->game.board, &app->game.undo, node.cl.moves[i])))
			continue;

		score = -quiescent(app, &node, -beta, -alpha);

		node.made++;
		undo_move(app->game.board, &app->game.undo);

		if (score > alpha) {
			if (score >= beta) {
				return beta;
			}

			/* update alpha */
			alpha = score;
		}
	}

	return alpha;
}


int alpha_beta(app_t *app, cnodeptr_t parent, int alpha, int beta, int depth)
{
	int palpha = alpha;
	int i, score = -MATE, highest = -MATE;
	node_t node;
	move_t cutoff = 0;
	piece_t p;

	init_node(&node);

	assert(app);

	app->search.nodes++;
	node.depth = depth;

	/* max depth */
	if (app->game.board->ply > (SEARCH_MAXDEPTH - 1)) {
		/* return evaluate(app->board); */
		return quiescent(app, parent, alpha, beta);
	}

	/* recursive base */
	if (depth == 0) {
		return evaluate(app->game.board);
	}

	/* draws */
	if (repetitions(app) || (app->game.board->half >= 100)) {
		return 0;
	}

	/* if we are checked, set the nodes checked flag */
	if (check(app->game.board, app->game.board->side)) {
		node.flags |= NODE_CHECK;
		/* extend our search by 1 depth if we are in check */
		/* NOTES: we may want to NOT extend our search here if the parent
		   is in check, because the means we already extended once */
		depth++;
	}

	/* TODO:
	     - NULL moves
	     - Late-move reduction
	     - Tactical extensions (pins & forks -> depth++)
	 */

	/* probe our table */
	if (probe_hash(&app->hash, app->game.board, &cutoff, &score, depth, alpha, beta) == TRUE) {
		app->hash.cut++;
		return score;
	}

	/* generate moves */
	generate_moves(app->game.board, &node.ml, &node.ml);

	/* reset score */
	score = -MATE;

	/* try to match our hash hit move */
	if (cutoff != 0) {
		for (i = 0; i < node.ml.count; i++) {
			if (node.ml.moves[i] == cutoff) {
				node.ml.scores[i] = 20000;
				break;
			}
		}
	}

	/* search negamax */
	for (i = 0; i < node.ml.count; i++) {
		/* get the next move ordered */
		next_move(i, &node.ml);

		if (!(do_move(app->game.board, &app->game.undo, node.ml.moves[i])))
			continue;

		score = -alpha_beta(app, &node, -beta, -alpha, depth - 1);

		node.made++;
		undo_move(app->game.board, &app->game.undo);

		/* score whatever is best so far */
		if (score > highest) {
			node.best = node.ml.moves[i];
			highest = score;

			/* update alpha */
			if (score > alpha) {
				if (score >= beta) {

					/* non-captures causing beta cutoffs (killers) */
					if (!is_capture(node.ml.moves[i])) {
						app->game.board->killers[1][app->game.board->ply] =
							app->game.board->killers[0][app->game.board->ply];
						app->game.board->killers[0][app->game.board->ply] = node.ml.moves[i];
					}

					/* store this beta in our transposition table */
					store_hash(&app->hash, app->game.board, node.best, beta, HASH_BETA, depth);

					return beta;
				}

				/* update alpha */
				alpha = score;

				/* update our history */
				if (!is_capture(node.best)) {
					p = app->game.board->pos.squares[move_from(node.best)];
					app->game.board->history[piece_color(p)][piece_type(p)][move_to(node.best)] += depth;
				}
			}
		}
	}

	/* check for checkmate or stalemate */
	if (!node.made) {
		if (node.flags & NODE_CHECK) {
			return -MATE + app->game.board->ply;
		} else {
			return 0;
		}
	}

	if (alpha != palpha) {
		/* store this as an exact, since we beat alpha */
		store_hash(&app->hash, app->game.board, node.best, highest, HASH_EXACT, depth);
	} else {
		/* store the current alpha */
		store_hash(&app->hash, app->game.board, node.best, alpha, HASH_ALPHA, depth);
	}

	return alpha;
}


static int find_move(app_t *app, move_t m)
{
	int i;
	movelist_t ml;
	generate_moves(app->game.board, &ml, &ml);

	for (i = 0; i < ml.count; i++) {
		if (ml.moves[i] == m) return TRUE;
	}

	return FALSE;
}


static int print_pv(app_t *app, int depth)
{
	int count = 0;
	piece_t p;
	u8 from;
	move_t mv;

	assert(app);
	assert(app->game.board);

#define VIA_PROBE
#ifdef VIA_PROBE
	/* get the current position */
	mv = probe_hash_move(&app->hash, app->game.board->key);
	
	while ((mv != 0) && (count < depth)) {
		
		if (find_move(app, mv)) {
			from = move_from(mv);
			p = app->game.board->pos.squares[from];

			if (do_move(app->game.board, &app->game.undo, mv)) {
				app->search.pv[count++] = mv;

				print_algebraic(p, mv);
				printf(" ");
			} else {
				break;
			}
		} else {
			break;
		}

		mv = probe_hash_move(&app->hash, app->game.board->key);
	}
#else
	while (app->search.pv[count]) {
		mv = app->search.pv[count++];
		
		if (find_move(app, mv)) {
			from = move_from(mv);
			p = app->board->pos.squares[from];

			if (do_move(app->board, &app->ul, mv)) {
				print_algebraic(p, mv);
				printf(" ");
			} else {
				break;
			}
		} else {
			break;
		}
	}
#endif
	
	while (app->game.board->ply) {
		undo_move(app->game.board, &app->game.undo);
	}
	
	return count;
}


static void print_uci_info(app_t *app, int depth, int score)
{
	ms_time_t tm;
	u64 i, s;

	get_current_tick(&tm);
	i = get_interval(&app->search.start, &tm);
	s = i / 1000;

	printf("info depth %d score cp %d time %lld nodes %lld nps %lld pv ",
		depth, score,
		i, app->search.nodes,
		/* we are calculating average nps here - to calculate true nps we should
		   only consider the nodes searched and time interval for this depth */
		(s != 0 ? app->search.nodes / s : app->search.nodes));
	print_pv(app, depth);
	printf("\n");
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
	node_t root;

	assert(app);
	assert(app->game.board);

	init_node(&root);
	root.flags |= NODE_ROOT;

	/* reset our node count board->ply */
	app->game.board->ply = 0;
	app->search.nodes = 0;
	app->hash.generations++;

	/* clear the stop bit */
	app->search.flags &= ~SEARCH_STOPPED;

	/* clear search heuristics */
	memset(app->search.pv, 0, SEARCH_MAXDEPTH * sizeof(move_t));
	memset(app->game.board->killers, 0, 2 * SEARCH_MAXDEPTH * sizeof(int));
	memset(app->game.board->history, 0, 2 * 6 * 64 * sizeof(int));

	/* set the start time */
	get_current_tick(&app->search.start);

	/* iterative deepening */
	for (i = 1; i < app->search.depth; i++) {
		root.depth = i;

		val = alpha_beta(app, &root, -MATE, MATE, i);

		if (app->mode == IUCI) {
			print_uci_info(app, i, val);
		}
	}
}
