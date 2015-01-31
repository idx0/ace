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
 *
 * This file parses UCI commands and sets UCI variables in the app.
 */

#include <stdio.h>

#include "ace_display.h"
#include "ace_types.h"
#include "ace_global.h"

static const char uci_delim[] = " \t\n\r";

void init_uci()
{
    printf("id name %s %s\n", ACE_NAME, ACE_VERSION);
    printf("id author %s\n", ACE_AUTHOR);
    printf("option name Hash type spin default 16 min 1 max 1024\n");
    printf("option name Nullmove type check default true\n");
    printf("option name Clear Hash type button\n");
    printf("uciok\n");
}


static int process_uci_move(app_t *app, char *sz, size_t len)
{
    move_t m;

    if (sz) {
		m = process_long_notation(app->game.board, sz, len, app->game.board->side);

        if (m) {
			do_move(app->game.board, &app->game.undo, m);
        } else {
        	return FALSE;
        }
    }

    return TRUE;
}


int parse_uci_position(app_t *app, char **ctx)
{
	fen_state_t fen;
	char fenbuf[256], *ptr;
	int fenlen = 0, j;
	size_t cmdlen;
	enum { CMDMODE, FENMODE, MOVEMODE } mode;

	mode = CMDMODE;

	while ((ptr = strtok2(NULL, uci_delim, ctx))) {
		cmdlen = strlen(ptr);

		if (mode == CMDMODE) {
			/* parse words as command options */
			if (strncmp(ptr, "fen", 3) == 0) {
				mode = FENMODE;
				fenlen = 0;
			} else if (strncmp(ptr, "startpos", 8) == 0) {
				init_startpos(app);
			} else if (strncmp(ptr, "moves", 5) == 0) {
				mode = MOVEMODE;
			}
		} else if (mode == FENMODE) {
			if (strncmp(ptr, "moves", 5) == 0) {
				mode = MOVEMODE;
				fenbuf[fenlen] = 0;
				fenlen++;

				fen_init(&fen);
				fen_use_ptr(&fen, app->game.board);
	            fen_parse(&fen, fenbuf, fenlen);
	            fen_destroy(&fen);
				app->game.undo.count = 0;
			} else {
				/* parse words as fen string pieces */
				for (j = 0; ((j < cmdlen) && (fenlen < 254)); j++, fenlen++) {
					fenbuf[fenlen] = ptr[j];
				}
				fenbuf[fenlen] = ' ';
				fenlen++;
			}
		} else if (mode == MOVEMODE) {
			/* parse words as long notation moves */
			process_uci_move(app, ptr, cmdlen);
		}
	}

	return TRUE;
}


static void parse_uci_go(app_t *app, char **ctx)
{
	char *ptr;

	while ((ptr = strtok2(NULL, uci_delim, ctx))) {
	}

	think(app);
}


int parse_uci(app_t *app, char *sz, size_t len)
{
	char *ptr, *ctx = NULL;

	ptr = strtok2(sz, uci_delim, &ctx);

	if (ptr) {

		if (strncmp(ptr, "stop", 4) == 0) {
		} else if (strncmp(ptr, "position", 8) == 0) {
			return parse_uci_position(app, &ctx);
		} else if (strncmp(ptr, "ucinewgame", 10) == 0) {
			init_startpos(app);
		} else if (strncmp(ptr, "go", 2) == 0) {
			parse_uci_go(app, &ctx);
		}
	}

	return TRUE;
}
