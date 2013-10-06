/**
 * thread_sync_latency
 * benchmarks for the latency of various inter-thread synchronization
 * primitives
 *
 * Written in 2013 by William Light <wrl@illest.net>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to the
 * public domain worldwide. This software is distributed without any
 * warranty. 
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 */

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#include "timeutil.h"
#include "report.h"
#include "test_params.h"

struct thread_sync {
	struct timespec send_time;
	useconds_t slept_for;
	int done;

	sem_t sem;
};

struct thread_sync tsync;

static void *
writer_thread(void *ctx)
{
	struct test_run *run;
	useconds_t sleep_for;
	int times, i = 0;

	for (run = test_runs; run->sleep_for; run++) {
		sleep_for = run->sleep_for;
		times = run->times;

		for (i = 0; i < times; i++) {
			usleep(sleep_for);
			tsync.slept_for = sleep_for;

			clock_gettime(CLOCK_MONOTONIC, &tsync.send_time);
			sem_post(&tsync.sem);
		}
	}

	usleep(sleep_for * 2);
	tsync.done = 1;
	sem_post(&tsync.sem);

	return NULL;
}

int
main(int argc, char **argv)
{
	struct timespec recv_time, diff;
	pthread_t writer;

	sem_init(&tsync.sem, 0, 0);
	tsync.done = 0;

	pthread_create(&writer, NULL, writer_thread, NULL);

	for (;;) {
		sem_wait(&tsync.sem);

		if (tsync.done)
			break;

		clock_gettime(CLOCK_MONOTONIC, &recv_time);
		timespec_diff(&diff, &recv_time, &tsync.send_time);

		report(tsync.slept_for, diff.tv_sec, diff.tv_nsec);
	}

	return EXIT_SUCCESS;
}