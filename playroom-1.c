#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "params.h"

#include "playroom.h"

// Initialization and destruction functions
int playroom_init(struct playroom *room)
{
	int rv;

	// Zero everything by default
	memset(room, 0, sizeof *room);

	// Initialize mutex and condition variables
	rv = pthread_mutex_init(&room->lock, NULL);
	rv &= pthread_cond_init(&room->dogs_out, NULL);
	rv &= pthread_cond_init(&room->cats_out, NULL);
	if (rv)
		return rv;
	
	return 0;
}

void playroom_destroy(struct playroom *room)
{
	while (pthread_mutex_destroy(&room->lock) == EBUSY);
}

// Entry/exit functions
void cat_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	while(room->dogs.n > 0) {
		room->cats.wait++;
		pthread_cond_wait(&room->dogs_out, &room->lock);
		room->cats.wait--;
	}
	room->cats.total++;
	room->cats.n++;
	pthread_mutex_unlock(&room->lock);
}

void cat_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->cats.n--;
	if (room->cats.n <= 0) {
		pthread_cond_broadcast(&room->cats_out);
	}
	pthread_mutex_unlock(&room->lock);
}

void dog_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	while(room->cats.n > 0) {
		room->dogs.wait++;
		pthread_cond_wait(&room->cats_out, &room->lock);
		room->dogs.wait--;
	}
	room->dogs.total++;
	room->dogs.n++;
	pthread_mutex_unlock(&room->lock);
}

void dog_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->dogs.n--;
	if (room->dogs.n <= 0) {
		pthread_cond_broadcast(&room->dogs_out);
	}
	pthread_mutex_unlock(&room->lock);
}

// Output functions
void playroom_print(struct playroom *room)
{
	// Use this so we get a consistent snapshot
	pthread_mutex_lock(&room->lock);
	printf("play: %2dc %2dd     wait: %2dc %2dd     total: %2dc %2dd\n",
			room->cats.n, room->dogs.n,
			room->cats.wait, room->dogs.wait,
			room->cats.total, room->dogs.total);
	pthread_mutex_unlock(&room->lock);
}

void playroom_visual(struct playroom *room)
{
	int i;

	// Print out a one-line "visual" view of the playroom state
	pthread_mutex_lock(&room->lock);
	for (i = NUM_CATS; i > room->cats.wait; i--)
		printf(" ");
	for (; i > 0; i--)
		printf("c");
	printf("|");
	for (i = 0; i < room->cats.n; i++)
		printf("c");
	for (; i < NUM_CATS; i++)
		printf(" ");

	for (i = NUM_DOGS; i > room->dogs.n; i--)
		printf(" ");
	for (; i > 0; i--)
		printf("d");
	printf("|");
	for (i = 0; i < room->dogs.wait; i++)
		printf("d");
	for (; i < NUM_DOGS; i++)
		printf(" ");

	printf("\n");
	pthread_mutex_unlock(&room->lock);
}
