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

#ifndef _DEBUG
int main(int argc, char **argv)
#else
int main2(int argc, char **argv)
#endif
{
	int i = 0, c;
	char buf[256];
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

		i = 0;
		fflush(stdout);
		while (i < 256) {
			c = getchar();

			if ((c == EOF) || (c == '\r') || (c == '\n')) {
				buf[i] = 0;
				break;
			}

			buf[i] = c;
			i++;
		}
		buf[i] = 0;

		/* process_command */
		if (!process_command(&app, buf, i)) {
			fprintf(stderr, "ace: unrecognized command string `%s'\n", buf);
		}
	}

	return 0;
}

