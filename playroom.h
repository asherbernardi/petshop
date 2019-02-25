#ifndef PLAYROOM_H
#define PLAYROOM_H

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CATS 1
#define DOGS 2
#define BIRDS 4
#define MICE 8
#define WOLVES 16
#define NONE 0
#define MAX_INT 2147483647

struct pet_counter {
	// Number currently in the room
	int n;
	// Number currently waiting outside
	int wait;
	// Time at which these pets started waiting
	int wait_time;
	// The condition variable associated with this pet
	pthread_cond_t *condition;
	// The presence number of the animals to avoid
	int avoid;

	// Number of useless wakeups
	int wakeup;
	// Number of successful entries
	int total;
};

struct playroom {
	pthread_mutex_t lock;
	pthread_cond_t dogs_birds_mice_wolves_out; // for cats
	pthread_cond_t cats_mice_wolves_out; // for dogs
	pthread_cond_t cats_wolves_out; // for birds
	pthread_cond_t cats_dogs_wolves_out; // for mice
	pthread_cond_t all_out; // for wolves

	// Animal counters
	struct pet_counter cats, dogs, birds, mice, wolves;

	// Total number of pets in the room
	int n;

	int time;

	// Flag that room is shutting down
	int ending;
};

int playroom_init(struct playroom *room);
void playroom_destroy(struct playroom *room);

void cat_enter(struct playroom *room);
void cat_exit(struct playroom *room);

void dog_enter(struct playroom *room);
void dog_exit(struct playroom *room);

void bird_enter(struct playroom *room);
void bird_exit(struct playroom *room);

void mouse_enter(struct playroom *room);
void mouse_exit(struct playroom *room);

void wolf_enter(struct playroom *room);
void wolf_exit(struct playroom *room);

void playroom_print(struct playroom *room);
void playroom_visual(struct playroom *room);

#endif
