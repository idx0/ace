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

void get_current_tick(ms_time_t* t)
{
	assert(t);
#ifdef ACE_WINDOWS
	t->time = GetTickCount64();
#else
	gettimeofday(&t->time, NULL);
#endif
}


u64 get_interval(const ms_time_t *then, const ms_time_t* now)
{
	assert(then);
	assert(now);
#ifdef ACE_WINDOWS
	return (now->time - then->time);
#else
	return ((now->time.tv_sec - then->time.tv_sec) * 1000) +
			((now->time.tv_usec - then->time.tv_usec) / 1000);
#endif
}


/* This function tries its hardest to provide an aligned malloc implementation
   (short of doing it myself with buffer memory). */
#ifdef ACE_WINDOWS
#include <malloc.h>
void *xmalloc(size_t sz, size_t align)
{
    return _aligned_malloc(sz, align);
}
void xfree(void *ptr)
{
    _aligned_free(ptr);
}
#else
void *xmalloc(size_t sz, size_t align)
{
#if (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOUCE >= 600)
    void *rc;
    if (posix_memalign(&rc, align, sz))
        return NULL;
    else
        return rc;
#elif defined (_ISOC11_SOUCE)
    return aligned_alloc(align, sz);
#else
    /* on glibc, malloc is always align=8 */
    return malloc(sz);
#endif
}
void xfree(void *ptr)
{
    free(ptr);
}
#endif
