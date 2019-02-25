#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "playroom.h"
#include "pet.h"

#include "params.h"

int main(int argc, char *argv[])
{
	int i;
	pthread_t cats[NUM_CATS], dogs[NUM_DOGS], birds[NUM_BIRDS], mice[NUM_MICE], wolves[NUM_WOLVES];
	struct playroom room;
	struct pet *p;

	// Seed the random number generator
	srand(time(NULL));

	// Initialize the playroom
	if (playroom_init(&room)) {
		fprintf(stderr, "Playroom initialization failed.\n");
		return 1;
	}

	// Create the pets
	for (i = 0; i < NUM_CATS; i++) {
		p = malloc(sizeof *p);
		if (p == NULL) {
			fprintf(stderr, "Could not allocate pets.\n");
			return 1;
		}
		p->room = &room;
		p->interval = PET_INTERVAL_US;
		p->enter = cat_enter;
		p->exit = cat_exit;
		pthread_create(&cats[i], NULL, pet_run, p);
	}
	for (i = 0; i < NUM_DOGS; i++) {
		p = malloc(sizeof *p);
		if (p == NULL) {
			fprintf(stderr, "Could not allocate pets.\n");
			return 1;
		}
		p->room = &room;
		p->interval = PET_INTERVAL_US;
		p->enter = dog_enter;
		p->exit = dog_exit;
		pthread_create(&dogs[i], NULL, pet_run, p);
	}
	for (i = 0; i < NUM_BIRDS; i++) {
		p = malloc(sizeof *p);
		if (p == NULL) {
			fprintf(stderr, "Could not allocate pets.\n");
			return 1;
		}
		p->room = &room;
		p->interval = PET_INTERVAL_US;
		p->enter = bird_enter;
		p->exit = bird_exit;
		pthread_create(&birds[i], NULL, pet_run, p);
	}
	for (i = 0; i < NUM_MICE; i++) {
		p = malloc(sizeof *p);
		if (p == NULL) {
			fprintf(stderr, "Could not allocate pets.\n");
			return 1;
		}
		p->room = &room;
		p->interval = PET_INTERVAL_US;
		p->enter = mouse_enter;
		p->exit = mouse_exit;
		pthread_create(&mice[i], NULL, pet_run, p);
	}
	for (i = 0; i < NUM_WOLVES; i++) {
		p = malloc(sizeof *p);
		if (p == NULL) {
			fprintf(stderr, "Could not allocate pets.\n");
			return 1;
		}
		p->room = &room;
		p->interval = PET_INTERVAL_US;
		p->enter = wolf_enter;
		p->exit = wolf_exit;
		pthread_create(&wolves[i], NULL, pet_run, p);
	}

	// Periodically print out the state of the simulation
	for (i = 0; i < SIM_LEN_US; i += SIM_STEP_US) {
		playroom_print(&room);
		usleep(SIM_STEP_US);
	}
	playroom_print(&room);

	// Stop the simulation
	room.ending = 1;
	for (i = 0; i < NUM_DOGS; i++) {
		pthread_join(dogs[i], (void **) &p);
		free(p);
	}
	for (i = 0; i < NUM_CATS; i++) {
		pthread_join(cats[i], (void **) &p);
		free(p);
	}
	for (i = 0; i < NUM_BIRDS; i++) {
		pthread_join(birds[i], (void **) &p);
		free(p);
	}
	for (i = 0; i < NUM_MICE; i++) {
		pthread_join(mice[i], (void **) &p);
		free(p);
	}
	for (i = 0; i < NUM_WOLVES; i++) {
		pthread_join(wolves[i], (void **) &p);
		free(p);
	}

	// Tear down the playroom object
	playroom_destroy(&room);

	return 0;
}
