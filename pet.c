#include <stdlib.h>
#include <unistd.h>

#include "playroom.h"
#include "params.h"

#include "pet.h"

void *pet_run(void *arg)
{
	struct pet *p = arg;
	while (!p->room->ending) {
		usleep(rand() % p->interval);
		p->enter(p->room);
		if (!p->room->ending)
			usleep(rand() % p->interval);
		p->exit(p->room);
	}
	return p;
}
