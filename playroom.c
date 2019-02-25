#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

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
	rv &= pthread_cond_init(&room->dogs_birds_mice_wolves_out, NULL);
	rv &= pthread_cond_init(&room->cats_mice_wolves_out, NULL);
	rv &= pthread_cond_init(&room->cats_wolves_out, NULL);
	rv &= pthread_cond_init(&room->cats_dogs_wolves_out, NULL);
	rv &= pthread_cond_init(&room->all_out, NULL);
	if (rv)
		return rv;

	// Associate each condition variable with a pet
	room->cats.condition = &room->dogs_birds_mice_wolves_out;
	room->dogs.condition = &room->cats_mice_wolves_out;
	room->birds.condition = &room->cats_wolves_out;
	room->mice.condition = &room->cats_dogs_wolves_out;
	room->wolves.condition = &room->all_out;

	// Associate pet with pets to avoid
	room->cats.avoid = DOGS + BIRDS + MICE + WOLVES;
	room->dogs.avoid = CATS + MICE + WOLVES;
	room->birds.avoid = CATS + WOLVES;
	room->mice.avoid = CATS + DOGS + WOLVES;
	room->wolves.avoid = DOGS + CATS + BIRDS + MICE + WOLVES;

	room->cats.wait_time = MAX_INT;
	room->dogs.wait_time = MAX_INT;
	room->birds.wait_time = MAX_INT;
	room->mice.wait_time = MAX_INT;
	room->wolves.wait_time = MAX_INT;

	return 0;
}

void playroom_destroy(struct playroom *room)
{
	while (pthread_mutex_destroy(&room->lock) == EBUSY);
}

// struct pet_counter *dequeue(struct queue *q) {
// 	struct pet_counter *ret;
// 	ret = q->front->pet;
// 	q->front->prev->next = NULL;
// 	q->front = q->front->prev;
// 	return ret;
// }

// void enqueue(struct queue *q, struct pet_counter *p) {
// 	struct node new = malloc(sizeof(node));
// 	new.prev = NULL;
// 	new.pet = p;
// 	new.next = q->back;
// 	q->back->prev = new;
// 	q->back = new;
// }

// find which animals have left the room
// MUST HAVE A MUTEX LOCK WHEN CALLING THIS FUNCTION
int get_presence(struct playroom *room) {
	int ret = 0;
	if (room->cats.n > 0)
		ret += CATS;
	if (room->dogs.n > 0)
		ret += DOGS;
	if (room->birds.n > 0)
		ret += BIRDS;
	if (room->mice.n > 0)
		ret += MICE;
	if (room->wolves.n > 0)
		ret += WOLVES;
	return ret;
}

int longest_waiting_pet_index(struct pet_counter *arr[], int n) {
	if (n == 1) return 0;
	int min = 0;
	for (int i = 1; i < n; i++) {
		if (arr[i]->wait_time < arr[i-1]->wait_time)
			min = i;
	}
	return min;
}

void delete_at_index(struct pet_counter *arr[], int n, int index) {
	for (int i = index + 1; i < n; i++) {
		arr[i - 1] = arr[i];
	}
}

// broadcasts only to the an animal that can go into the room,
// prioritizing last to begin waiting
void auto_broadcast(struct playroom *room) {
	int presence = get_presence(room);
	struct pet_counter *arr[5] = {&room->cats, &room->dogs, &room->birds, &room->mice, &room->wolves};
	for (int n = 5; n > 0; n--) {
		int next = longest_waiting_pet_index(arr, n);
		if ((arr[next]->wait > 0) && !(presence & arr[next]->avoid)) {
			// we want to just signal to the wolves.
			if (next == 4) pthread_cond_signal(arr[next]->condition);
			else pthread_cond_broadcast(arr[next]->condition);
			return;
		}
		delete_at_index(arr, n, next);
	}
}

// Entry/exit functions
void cat_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	int wakes = 0;
	int presence = get_presence(room);
	while (presence & room->cats.avoid) {
		if (wakes > 0)
			room->cats.wakeup++; // useless wakeup
		// No cat has been waiting, set the time of the first waiting cat
		if (room->cats.wait_time == MAX_INT)
			room->cats.wait_time = room->time++;
		room->cats.wait++;
		pthread_cond_wait(room->cats.condition, &room->lock);
		room->cats.wait--;
		wakes++;
		presence = get_presence(room);
	}
	// Now that the cats have gotten in, they are no longer waiting
	room->cats.wait_time = MAX_INT;
	room->cats.total++;
	room->cats.n++;
	pthread_mutex_unlock(&room->lock);
}

void cat_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->cats.n--;
	// A condition has only changed if there are no more cats in the room
	if (room->cats.n <= 0)
		auto_broadcast(room);

	pthread_mutex_unlock(&room->lock);
}

void dog_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	int wakes = 0;
	int presence = get_presence(room);
	while (presence & room->dogs.avoid) {
		if (wakes > 0)
			room->dogs.wakeup++;
		if (room->dogs.wait_time == MAX_INT)
			room->dogs.wait_time = room->time++;
		room->dogs.wait++;
		pthread_cond_wait(room->dogs.condition, &room->lock);
		room->dogs.wait--;
		wakes++;
		presence = get_presence(room);
	}
	room->dogs.wait_time = MAX_INT;
	room->dogs.total++;
	room->dogs.n++;
	pthread_mutex_unlock(&room->lock);
}

void dog_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->dogs.n--;
	// A condition has only changed if there are no more dogs in the room
	if (room->dogs.n <= 0)
		auto_broadcast(room);

	pthread_mutex_unlock(&room->lock);
}

void bird_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	int wakes = 0;
	int presence = get_presence(room);
	while (presence & room->birds.avoid) {
		if (wakes > 0)
			room->birds.wakeup++;
		if (room->birds.wait_time == MAX_INT)
			room->birds.wait_time = room->time++;
		room->birds.wait++;
		pthread_cond_wait(room->birds.condition, &room->lock);
		room->birds.wait--;
		wakes++;
		presence = get_presence(room);
	}
	room->birds.wait_time = MAX_INT;
	room->birds.total++;
	room->birds.n++;
	pthread_mutex_unlock(&room->lock);
}

void bird_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->birds.n--;
	// A condition has only changed if there are no more birds in the room
	if (room->birds.n <= 0)
		auto_broadcast(room);

	pthread_mutex_unlock(&room->lock);
}

void mouse_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	int wakes = 0;
	int presence = get_presence(room);
	while (presence & room->mice.avoid) {
		if (wakes > 0)
			room->mice.wakeup++;
		if (room->mice.wait_time == MAX_INT)
			room->mice.wait_time = room->time++;
		room->mice.wait++;
		pthread_cond_wait(room->mice.condition, &room->lock);
		room->mice.wait--;
		wakes++;
		presence = get_presence(room);
	}
	room->mice.wait_time = MAX_INT;
	room->mice.total++;
	room->mice.n++;
	pthread_mutex_unlock(&room->lock);
}

void mouse_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->mice.n--;
	// A condition has only changed if there are no more mice in the room
	if (room->mice.n <= 0)
		auto_broadcast(room);
	
	pthread_mutex_unlock(&room->lock);
}

void wolf_enter(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	int wakes = 0;
	int presence = get_presence(room);
	while (presence & room->wolves.avoid) {
		if (wakes > 0)
			room->wolves.wakeup++;
		if (room->wolves.wait_time == MAX_INT)
			room->wolves.wait_time = room->time++;
		room->wolves.wait++;
		pthread_cond_wait(room->wolves.condition, &room->lock);
		room->wolves.wait--;
		wakes++;
		presence = get_presence(room);
	}
	room->wolves.wait_time = MAX_INT;
	room->wolves.total++;
	room->wolves.n++;
	pthread_mutex_unlock(&room->lock);
}

void wolf_exit(struct playroom *room)
{
	pthread_mutex_lock(&room->lock);
	room->wolves.n--;
	// A condition has only changed if there are no more wolves in the room
	if (room->wolves.n <= 0)
		auto_broadcast(room);

	pthread_mutex_unlock(&room->lock);
}

// Output functions
void playroom_print(struct playroom *room)
{
	// Use this so we get a consistent snapshot
	pthread_mutex_lock(&room->lock);
	printf("play: %2dc %2dd %2db %2dm %2dw     wait: %2dc %2dd %2db %2dm %2dw     total: %2dc %2dd %2db %2dm %2dw     useless wakeups: %2dc %2dd %2db %2dm %2dw\n",
			room->cats.n, room->dogs.n, room->birds.n, room->mice.n, room->wolves.n,
			room->cats.wait, room->dogs.wait, room->birds.wait, room->mice.wait, room->wolves.wait,
			room->cats.total, room->dogs.total, room->birds.total, room->mice.total, room->wolves.total,
			room->cats.wakeup, room->dogs.wakeup, room->birds.wakeup, room->mice.wakeup, room->wolves.wakeup);
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

	for (i = NUM_DOGS + NUM_BIRDS; i > room->dogs.n + room->birds.n; i--)
		printf(" ");
	for (; i > room->birds.n; i--)
		printf("d");
	for (; i > 0; i--)
		printf("b");
	printf("|");
	for (i = 0; i < room->dogs.wait; i++)
		printf("d");
	for (; i < room->dogs.wait + room->birds.wait; i++)
		printf("b");
	for (; i < NUM_DOGS + NUM_BIRDS; i++)
		printf(" ");

	printf("\n");
	pthread_mutex_unlock(&room->lock);
}
