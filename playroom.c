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

	rv = pthread_mutex_init(&room->lock, NULL);
	if (rv)
		return rv;
	
	// Init semaphores
	sem_init(&room->dogs_in, 0, 1);
	sem_init(&room->dogs_in, 0, 1);

	return 0;
}

void playroom_destroy(struct playroom *room)
{
	while (pthread_mutex_destroy(&room->lock) == EBUSY);
}

// Entry/exit functions
void cat_enter(struct playroom *room)
{
	if (room->cats.n == 0) {
		sem_wait(&room->dogs_in);
	}
	room->cats.n++;
	room->cats.total++;




	// make sure the cat can get in
	sem_wait(&room->cats_in);
	// increment cat numbers
	room->cats.n++;
	room->cats.total++;
	// if this is the first cat in, lock the dogs
	if (room->cats.n == 1) {
		sem_wait(&room->dogs_in);
	}
	// let more cats in
	sem_post(&room->cats_in);
}

void cat_exit(struct playroom *room)
{
	





	// Make sure no other cat is leaving at the same time
	sem_wait(&room->cats_in);
	room->cats.n--;
	// if this is the last cat to leave, unlock the dogs
	if (room->cats.n == 0) {
		sem_post(&room->dogs_in);
	}
	// Once a cat leaves, signal the other cats that they can enter
	sem_post(&room->cats_in);
}

void dog_enter(struct playroom *room)
{
	// make sure the dog can get in
	sem_wait(&room->dogs_in);
	// increment dog numbers
	room->dogs.n++;
	room->dogs.total++;
	// if this is the first dog in, lock the cats
	if (room->dogs.n == 1) {
		sem_wait(&room->cats_in);
	}
	// let more dogs in
	sem_post(&room->dogs_in);
}

void dog_exit(struct playroom *room)
{
	
	// Make sure no other dog is leaving at the same time
	sem_wait(&room->dogs_in);
	room->dogs.n--;
	// if this is the last dog to leave, unlock the cats
	if (room->dogs.n == 0) {
		sem_post(&room->cats_in);
	}
	// Once a dog leaves, signal the other dogs that they can enter
	sem_post(&room->dogs_in);
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
