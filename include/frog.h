#ifndef FROG_H_
#define FROG_H_
#include "actorPatternFramework.h"
#include "frog-functions.h"
#include "messages.h"


typedef struct Frog{
	Actor * base;
	int cellIDs[16];
	int clockId;
	float x;
	float y;
	long state;
	int infected;
	int hop_counter_birth;
	int hop_counter_death;
	int hop_counter_catch;
	int sum_pop_influx;
	int infection_sum;
	int *infection_history;
}Frog;

Actor * newFrog();
void deleteFrog(Frog *);
#endif