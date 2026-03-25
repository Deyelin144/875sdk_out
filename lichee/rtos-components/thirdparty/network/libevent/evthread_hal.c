/*
 * Copyright 2009-2012 Niels Provos and Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "event2/event-config.h"
#include "evconfig-private.h"

#include "tc_iot_hal.h"

struct event_base;
#include "event2/thread.h"
#include "mm-internal.h"
#include "evthread-internal.h"

/*
 * evthread functions using HAL as thread/mutex/cond access
 */

static void *
evthread_hal_lock_alloc(unsigned locktype)
{
	// currently only EVTHREAD_LOCKTYPE_RECURSIVE is supported
	if (locktype & EVTHREAD_LOCKTYPE_READWRITE) {
		event_warn("invalid lock type");
		return NULL;
	}

	void *lock = HAL_RecursiveMutexCreate();
	event_msgx("lock allocated type %u %p", locktype, lock);
	return lock;
}

static void
evthread_hal_lock_free(void *lock_, unsigned locktype)
{
	if (!lock_) {
		event_warn("invalid lock handle");
		return;
	}
	HAL_RecursiveMutexDestroy(lock_);
	event_msgx("lock free type %u %p", locktype, lock_);
}

static int
evthread_hal_lock(unsigned mode, void *lock_)
{
	if (!lock_) {
		event_warn("invalid lock handle");
		return -1;
	}

    int try = 0;
    if (mode & EVTHREAD_TRY) {
        try = 1;
    }

	if (HAL_RecursiveMutexLock(lock_, try)) {
		event_msgx("MutexLock failed");
		return -1;
	}

	// event_debug(("evthread_hal_lock mode %u %p", mode, lock_));
	return 0;
}

static int
evthread_hal_unlock(unsigned mode, void *lock_)
{
	if (!lock_) {
		event_warn("invalid lock handle");
		return -1;
	}

	if (HAL_RecursiveMutexUnLock(lock_)) {
		event_msgx("MutexUnLock failed");
		return -1;
	}

	// event_debug(("evthread_hal_unlock mode %u %p", mode, lock_));
	return 0;
}

static unsigned long
evthread_hal_get_id(void)
{
	unsigned long handle = (unsigned long)HAL_GetCurrentThreadHandle();
	event_debug(("current task handle %lu", handle));
	return handle;
}

static void *
evthread_hal_cond_alloc(unsigned condflags)
{
	void *pCond = HAL_CondCreate();
	event_msgx("cond allocated flag %u %p", condflags, pCond);
	return pCond;
}

static void
evthread_hal_cond_free(void *cond_)
{
	if (!cond_) {
		event_warn("invalid cond handle");
		return;
	}

	HAL_CondFree(cond_);
	event_msgx("cond free %p", cond_);
}

static int
evthread_hal_cond_signal(void *cond_, int broadcast)
{
	if (!cond_) {
		event_warn("invalid cond handle");
		return -1;
	}

	if (HAL_CondSignal(cond_, broadcast)) {
		event_msgx("Cond_Signal faild %p", cond_);
		return -1;
	}

	// event_debug(("evthread_hal_cond_signal mode %u %p", broadcast, cond_));
	return 0;
}

static int
evthread_hal_cond_wait(void *cond_, void *lock_, const struct timeval *tv)
{
	if (!cond_ || !lock_) {
		event_warn("invalid cond or lock handle");
		return -1;
	}

    // 0 means wait forever
    unsigned long wait_ms = 0;
    if (tv) {
		struct timeval now, abstime;
		struct timespec ts;
		evutil_gettimeofday(&now, NULL);
		evutil_timeradd(&now, tv, &abstime);
		wait_ms = abstime.tv_sec*1000;
		wait_ms += abstime.tv_usec/1000;
	}

    int ret = HAL_CondWait(cond_, lock_, wait_ms);
	if (ret) {
		event_msgx("Cond_Signal %p failed %d", cond_, ret);
		return -1;
	}

	return 0;
}

int
evthread_use_hal(void)
{
	struct evthread_lock_callbacks cbs = {EVTHREAD_LOCK_API_VERSION,
		EVTHREAD_LOCKTYPE_RECURSIVE, evthread_hal_lock_alloc,
		evthread_hal_lock_free, evthread_hal_lock, evthread_hal_unlock};
	struct evthread_condition_callbacks cond_cbs = {
		EVTHREAD_CONDITION_API_VERSION, evthread_hal_cond_alloc,
		evthread_hal_cond_free, evthread_hal_cond_signal,
		evthread_hal_cond_wait};

	evthread_set_lock_callbacks(&cbs);
	evthread_set_condition_callbacks(&cond_cbs);
	evthread_set_id_callback(evthread_hal_get_id);

	return 0;
}
