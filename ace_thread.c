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

#include "ace_thread.h"


int mutex_lock(mutex_t *mutex)
{
#ifdef ACE_WINDOWS
	EnterCriticalSection(&mutex->mutex);
	return 0;
#else /* ACE_UNIX */
	return pthread_mutex_lock(&mutex->mutex);
#endif
}

int mutex_unlock(mutex_t *mutex)
{
#ifdef ACE_WINDOWS
	LeaveCriticalSection(&mutex->mutex);
	return 0;
#else /* ACE_UNIX */
	return pthread_mutex_unlock(&mutex->mutex);
#endif
}

int mutex_trylock(mutex_t *mutex)
{
#ifdef ACE_WINDOWS
	return (TryEnterCriticalSection(&mutex->mutex) ? 0 : EBUSY);
#else /* ACE_UNIX */
	return pthread_mutex_trylock(&mutex->mutex);
#endif
}

int mutex_init(mutex_t *mutex)
{
#ifdef ACE_WINDOWS
	InitializeCriticalSection(&mutex->mutex);
	return 0;
#else /* ACE_UNIX */
	return pthread_mutex_init(&mutex->mutex, NULL);
#endif
}

int mutex_destroy(mutex_t *mutex)
{
#ifdef ACE_WINDOWS
	DeleteCriticalSection(&mutex->mutex);
	return 0;
#else /* ACE_UNIX */
	return pthread_mutex_destroy(&mutex->mutex);
#endif
}


/** num_cpus
	Note: may need to set or check some feature_test_macros to make this work.

	This function will return -1 if it cannot determine on your PC.
*/
int num_cpus()
{
	int ncpus = -1;
#ifdef ACE_WINDOWS
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	/* returns # of logical processors */
	ncpus = (int)info.dwNumberOfProcessors;
#else
#  if defined(_SC_NPROCESSORS_ONLN) && defined(_SC_NPROCESSORS_CONF)
	ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (sysconf(_SC_NPROCESSORS_CONF) != sysconf(_SC_NPROCESSORS_ONLN)) {
		fprintf(stderr, "dpl: warning: number of configured processors != "
			"number of available\nprocessors");
	}
#  endif
#endif

	return ncpus;
}


#include <string.h>
#include <stdlib.h>
#include <errno.h>
/**
 * When using _beginthreadex, callbacks use the __stdcall calling convention.
 * Additionally, they return unsigned rather than void - so for win32 we must
 * use a wrapper.
 *
 * We can use a similar wrapper on linux to get the thread specific pid.
 */
thread_control_t _tls_control = ACE_THREAD_DEFAULT_CONTROL;

#ifdef ACE_WINDOWS
DWORD _tls;
/** _beginthreadex, _endthreadex, _ReadWriteBarrier */
#include <process.h>
#include <intrin.h>
#else
pthread_key_t _tls;
#endif

static void internal_tls_init(void)
{
#ifdef ACE_WINDOWS
	_tls = TlsAlloc();
	if (_tls == TLS_OUT_OF_INDEXES) exit(ENOMEM);
#else
	pthread_key_create(&_tls, NULL);
#endif
}
 
#ifdef ACE_WINDOWS
static unsigned __stdcall internal_win32_callback_wrapper(void *parg)
{
	thread_t *t = (thread_t *)parg;
	
	thread_once(&_tls_control, internal_tls_init);
	TlsSetValue(_tls, t);
	
	if (t) { t->callback(parg); }
	
	while (t->thread == (HANDLE)-1) {
		YieldProcessor();
		_ReadWriteBarrier();
	}
	
	return 0;
}
#else
static void *internal_linux_callback_wrapper(void *parg)
{
	thread_t *t = (thread_t *)parg;
	
	thread_once(&_tls_control, internal_tls_init);
	pthread_setspecific(_tls, t);
	
	if (t) {
		t->thread_id = getpid();
		t->callback(parg);
	}
	
	pthread_exit(NULL);
}
#endif
/** end wrappers */

int thread_cancel(thread_t *thread)
{
#ifdef ACE_WINDOWS
	/** TODO: Add cleanup on windows */
	fprintf(stderr, "ace: error: thread cancel unsupported on win32 systems.");

	return 0;
#else
	thread->flags |= ACE_THREAD_CANCELLED;
	return pthread_cancel(thread->thread);
#endif
}

int thread_once(thread_control_t *control, void (*callback)(void))
{
#ifdef ACE_WINDOWS
	int state = (int)(*control);
	
	_ReadWriteBarrier();
	
	while (state != 1) {
		if ((!state) && (!_InterlockedCompareExchange(control, 2, 0))) {
			callback();
			*control = 1;
			
			return 0;
		}
		
		YieldProcessor();
		_ReadWriteBarrier();
		
		state = (int)(*control);
	}
	
	return 0;
#else
	return pthread_once(control, callback);
#endif
}

int thread_detach(thread_t *thread)
{
#ifdef ACE_WINDOWS
	CloseHandle(thread->thread);
	_ReadWriteBarrier();
	thread->thread = 0;
	
	return 0;
#else
	return pthread_detach(thread->thread);
#endif
}

int thread_join(thread_t *thread, void **result)
{
	if (!(thread->flags & ACE_THREAD_JOINABLE)) return 1;
	
#ifdef ACE_WINDOWS	
	WaitForSingleObject(thread->thread, INFINITE);
	CloseHandle(thread->thread);
	
	if (result) { *result = thread->result; }
	
	return 0;
#else
	return pthread_join(thread->thread, result);
#endif
}

void thread_exit(void *result)
{
	thread_t *t = thread_self();
	t->result = result;
	
#ifdef ACE_WINDOWS
	_endthreadex(0);
#else
	pthread_exit(NULL);
#endif
}

/**
 * The wrapper calls for thread creation attempt to store the thread_t 
 * pointer in thread-local storage.  When thread_self is called that
 * data is retreived (if it can be), otherwise an empty thread_t structure
 * is created and filled with the proper handle.
 */
thread_t *thread_self()
{
	thread_t *t;
	
	thread_once(&_tls_control, internal_tls_init);
#ifdef ACE_WINDOWS
	t = (thread_t *)TlsGetValue(_tls);
	
	if (!t)
	{
		t = (thread_t *)malloc(sizeof(thread_t));
		
		if (!t) exit(ENOMEM);
		
		memset(t, 0, sizeof(thread_t));
		t->thread = GetCurrentThread();
		
		TlsSetValue(_tls, t);
	}
#else
	t = (thread_t *)pthread_getspecific(_tls);

	if (!t)
	{
		t = malloc(sizeof(thread_t));
		
		if (!t) exit(ENOMEM);
		
		memset(t, 0, sizeof(thread_t));
		t->thread = pthread_self();
		
		pthread_setspecific(_tls, t);
	}
#endif

	return t;
}

void thread_create(thread_t *thread)
{
#ifdef ACE_WINDOWS
	thread->thread = (HANDLE)-1;
	_ReadWriteBarrier();
	
	thread->thread = (HANDLE)_beginthreadex(NULL, 0,
		&internal_win32_callback_wrapper, NULL, 0, &thread->thread_id);
#else
	pthread_attr_init(&thread->thread_attr);
	if (thread->flags & ACE_THREAD_JOINABLE) {
		pthread_attr_setdetachstate(&thread->thread_attr,
			PTHREAD_CREATE_JOINABLE);
	}
	
	if (pthread_create(&thread->thread, &thread->thread_attr,
		&internal_linux_callback_wrapper, (void *)thread)) {
		
		exit(ENOMEM);
	}
	
	pthread_attr_destroy(&thread->thread_attr);
#endif

	if (thread->flags & ACE_THREAD_DETACHED) {
		thread_detach(thread);
	}
}

int thread_init(
	thread_t *thread,
	callback_t callback,
	unsigned int attr)
{
	int rc = 1;

	if (thread) {
		memset(thread, 0, sizeof(thread_t));
	
		thread->flags = attr;
		thread->callback = callback;
		thread->result = NULL;
		
		thread->meta_size = 0;
		thread->meta = NULL;
		
#ifdef ACE_WINDOWS
		thread->thread = (HANDLE)-1;
		thread->thread_id = 0;
#else
		thread->thread_id = getpid();
#endif
	}
	
	return rc;
}

void thread_destroy(thread_t *thread)
{
	memset(thread, 0, sizeof(thread_t));
}
