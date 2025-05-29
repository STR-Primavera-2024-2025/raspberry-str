/* Copyright (C) 2024 Canonical Ltd

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Build with gcc edf.c -o edf */

#define _GNU_SOURCE

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

#define FIRST_CORE 0

struct sched_attr {
  uint32_t size;
  uint32_t sched_policy;
  uint64_t sched_flags;
  int32_t sched_nice;
  uint32_t sched_priority;
  uint64_t sched_runtime;
  uint64_t sched_deadline;
  uint64_t sched_period;
};

struct sched_attr attr1 = {
    .size = sizeof(struct sched_attr),
    .sched_policy = SCHED_DEADLINE,
    .sched_runtime = 10 * 1000 * 1000,
    .sched_deadline = 11 * 1000 * 1000,
    //.sched_period = 2 * 1000 * 1000 * 1000,
    .sched_period = 22 * 1000 * 1000,
};

struct sched_attr attr2 = {
    .size = sizeof(struct sched_attr),
    .sched_policy = SCHED_DEADLINE,
    .sched_runtime = 10 * 1000 * 1000,
    .sched_deadline = 11 * 1000 * 1000,
    //.sched_period = 2 * 1000 * 1000 * 1000,
    .sched_period = 22 * 1000 * 1000,
};

#include <time.h>
#include <errno.h>    

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

uint64_t get_ms() {
	struct timespec now;
	clock_gettime( CLOCK_REALTIME, &now );
	return (uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U;
}

int calls_on_thread1 = 0;
int calls_on_thread2 = 0;
#define SAMPLE_SIZE 256
uint64_t samples[SAMPLE_SIZE][2];

void *thread_start(void *arg) {
  int thread_num = (intptr_t)arg-1;


  for (uint64_t i = 0; i < SAMPLE_SIZE; i++) {
  	uint64_t start = get_ms();
    if (thread_num == 0) {
      ++calls_on_thread1;
    } else {
      ++calls_on_thread2;
    }

    while (get_ms() - start < 10);

    msleep(22 - (get_ms() - start));
    
    uint64_t end = get_ms();
    samples[i][thread_num] = end;
    // printf("%lu\n", samples[i][thread_num] - start);
  }

  return (void *)NULL;
}

int main(void) {
  pthread_t thread1, thread2; /* Thread IDs */

  /*-- Run on first core only --*/

  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(FIRST_CORE, &cpu_set);
  if (0 != sched_setaffinity(0, sizeof(cpu_set), &cpu_set)) {
    printf("Failed to set CPU affinity\n");
    exit(EXIT_FAILURE);
  }

  /*-- Call thread function on separate threads --*/
  uint64_t program_start = get_ms();
  
  if (0 != pthread_create(&thread1, NULL, &thread_start, (void *)(intptr_t)1)) {
    printf("Failed to create thread1\n");
    exit(EXIT_FAILURE);
  }

  if (0 != pthread_create(&thread2, NULL, &thread_start, (void *)(intptr_t)2)) {
    printf("Failed to create thread2\n");
    exit(EXIT_FAILURE);
  }

  /* Ensure both threads are allowed to complete */
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("thread1:   period =  %ld s\n",
         attr1.sched_period / (1000 * 1000 * 1000));
  printf("          runtime = %ld ms\n", attr1.sched_runtime / (1000 * 1000));
  printf("         deadline = %ld ms\n", attr1.sched_deadline / (1000 * 1000));
  printf("thread2:   period =  %ld s\n",
         attr2.sched_period / (1000 * 1000 * 1000));
  printf("          runtime = %ld ms\n", attr2.sched_runtime / (1000 * 1000));
  printf("         deadline = %ld ms\n\n",
         attr2.sched_deadline / (1000 * 1000));

  printf("Calls made on thread1: %d\nCalls made on thread2: %d\n",
         calls_on_thread1, calls_on_thread2);

  for (uint64_t i = 0; i < SAMPLE_SIZE; i++) {
  	printf("thread1: %lu\t", samples[i][0] - program_start);
  	printf("thread2: %lu\t", samples[i][1] - program_start);
  	printf("difference: %lu\n\n", samples[i][1] - samples[i][0]);
  }

  exit(EXIT_SUCCESS);
}
