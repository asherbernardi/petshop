#ifndef PLAYROOM_H
#define PLAYROOM_H

#include <pthread.h>
#include <semaphore.h>

struct pet_counter {
	// Number currently in the room
	int n;
	// Number currently waiting outside
	int wait;

	// Number of useless wakeups
	int wakeup;
	// Number of successful entries
	int total;
};

struct playroom {
	pthread_mutex_t lock;
	pthread_cond_t dogs_out;
	pthread_cond_t cats_out;

	// Animal counters
	struct pet_counter cats, dogs;

	// Flag that room is shutting down
	int ending;
};

int playroom_init(struct playroom *room);
void playroom_destroy(struct playroom *room);

void cat_enter(struct playroom *room);
void cat_exit(struct playroom *room);

void dog_enter(struct playroom *room);
void dog_exit(struct playroom *room);

void playroom_print(struct playroom *room);
void playroom_visual(struct playroom *room);

#endif
