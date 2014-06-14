#ifndef IODRIVER_H_
#define IODRIVER_H_



#include "actorPatternFramework.h"
#include "messages.h"


typedef struct IODriver{
	Actor * base;

	FILE * cells_file;
	int year;
	int num_cells;
	int counter;
	int * cellsInfo;
}IODriver;


Actor * newIODriver();
void deleteIODriver(Actor *);

#endif