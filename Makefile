OS = $(shell uname)

ifeq ($(OS),SunOS)
CFLAGS += -mt
endif

main: main.c params.h playroom.o pet.o
	$(CC) $(CFLAGS) -o $@ $< playroom.o pet.o -lpthread

pet.o: pet.c pet.h params.h playroom.h
	$(CC) $(CFLAGS) -c -o $@ $<

playroom.o: playroom.c playroom.h params.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) main main.o pet.o playroom.o
