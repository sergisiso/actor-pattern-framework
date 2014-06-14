#ifndef CLOCK_H
#define CLOCK_H

#include "actorPatternFramework.h"
#include "messages.h"
#include <time.h>
#include "unistd.h"

typedef struct Clock_{
	Actor * base;
	FILE * output;

  	int ini_frogs;
  	int max_frogs;
 	int number_cells;
 	int ini_inf_level;
	int num_years;


	double last_t; 
	int current_year;

	int frogs_io_counter;
	int year_time_counter;

  	int current_frogs;
	int * cellID;
	int * frogID;
	int IODriverID;
	int rtClockID;
}Clock;

Actor * newClock();
void deleteClock(Actor *);

#endif
