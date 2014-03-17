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

#pragma once

#include "ace_intrin.h"

#ifdef ACE_UNIX
# include <unistd.h>
# include <sys/types.h>
# include <pthread.h>
#endif

/**
 * On Linux:
 * This library requires NTPL (which is somewhat standard in gnu libc by now).
 * Nonetheless, a compiler check for pthread_setaffinity_np() should be added
 * to ensure DPL has proper thread affinity support for its thread pools.
 */

#ifndef EBUSY
#define EBUSY 16
#endif

/**
 * Returns the number of logical, active CPUs or -1 on failure
 */
extern int num_cpus();

/** --- mutex.h ---
 * These functions provide wrappers for a generic, cross-platform (windows and
 * unix) mutex API
 */
typedef struct mutex
{
#ifdef ACE_WINDOWS
	CRITICAL_SECTION mutex;
#else /* ACE_UNIX */
	pthread_mutex_t mutex;
#endif
} mutex_t;

extern int mutex_init(mutex_t *mutex);
extern int mutex_destroy(mutex_t *mutex);

extern int mutex_lock(mutex_t *mutex);
extern int mutex_unlock(mutex_t *mutex);
extern int mutex_trylock(mutex_t *mutex);

typedef void (*callback_t)(void *);

#define ACE_THREAD_CANCELLED   0x000100
#define ACE_THREAD_DETACHED    0x000001
#define ACE_THREAD_JOINABLE    0x000002
#define ACE_THREAD_AFFINITY    0x000004
#define ACE_THREAD_BARRIER     0x000008

typedef struct thread
{
	callback_t callback;
	
#ifdef ACE_WINDOWS
	HANDLE thread;
	unsigned thread_id;
#else
	pthread_t thread;
	pthread_attr_t thread_attr;
	pid_t thread_id;
#endif
	unsigned int flags;
	void *result;

	size_t meta_size;
	void *meta;
} thread_t;

#define thread_size() (sizeof(thread_t) - \
	(sizeof(size_t) + sizeof(void *)))
	
#ifdef ACE_WINDOWS
typedef unsigned long thread_control_t;
# define ACE_THREAD_DEFAULT_CONTROL 0
#else
typedef pthread_once_t thread_control_t;
# define ACE_THREAD_DEFAULT_CONTROL PTHREAD_ONCE_INIT
#endif
	
/**
 * The majority of these functions are designed to behave identically to the
 * call from the pthreads library of the similar name (whether on linux or
 * windows).
 */
extern int thread_init(
	thread_t *thread,
	callback_t callback,
	unsigned int attr);
extern void thread_destroy(thread_t *thread);

extern void thread_create(thread_t *thread);
extern void thread_exit(void *result);
extern thread_t *thread_self();

extern int thread_cancel(thread_t *thread);  /** Linux only */
extern int thread_once(thread_control_t *control, void (*callback)(void));
extern int thread_join(thread_t *thread, void **result);
extern int thread_detach(thread_t *thread);

