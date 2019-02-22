#ifndef PET_H
#define PET_H

#include "playroom.h"

struct pet {
	struct playroom *room;
	void (*enter)(struct playroom *);
	void (*exit)(struct playroom *);
	int interval;
};

void *pet_run(void *arg);

#endif
