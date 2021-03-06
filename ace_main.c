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

#include <stdio.h>
#include <assert.h>

#include "ace_types.h"
#include "ace_global.h"
#include "ace_display.h"
#include "ace_fen.h"
#include "ace_zobrist.h"
#include "ace_magic.h"

int main(int argc, char **argv)
{
	int i = 0, c;
	cmd_string_t* cmd = cmdstring_create(MAX_CMD_LEN);
	app_t app;

	init_zobrist(0xd4e12c77);
	init_magic();
	init_movelists();

	init_app(&app);

	printf("%s v%s - %s\n", ACE_NAME, ACE_VERSION, ACE_DESC);
	printf("%s %s\n", ACE_AUTHOR, ACE_EMAIL);

	while (!app.quit) {
		switch (app.mode) {
			case IACE:
				printf("\nace> ");
				break;
			case IMOVE:
				printf("\nace move> ");
				break;
			case IFEN:
				printf("\nace fen> ");
				break;
			default: break;
		}

		cmdstring_reset(cmd);

		i = 0;
		fflush(stdout);
		while (i < MAX_CMD_LEN) {
			c = getchar();

			if ((c == EOF) || (c == '\r') || (c == '\n')) {
				break;
			}

			cmdstring_add_to_buffer(cmd, c);
			i++;
		}

		/* process_command */
		if (!process_command(&app, cmd)) {
			/* fprintf(stderr, "ace: unrecognized command string `%s'\n", buf); */
		}
	}

	destroy_app(&app);

	cmdstring_destroy(cmd);

	return 0;
}

