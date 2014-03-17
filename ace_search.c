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

/**
 * The search is controlled by the think() function.  This function initializes
 * all variables needed for search, sets up thread pools, and then performs a
 * search starting at the root node.  A root node search is fundamentally
 * different from searches at other depths because we know for sure that we
 * should not perform a qsearch or a null-move search.
 */
void think()
{

}
