#ifndef CELL_H
#define CELL_H

#include "actorPatternFramework.h"
#include "messages.h"


typedef struct Cell_{
	Actor * base;
	int CellNum;
	int infection_level;
	int population_influx;
	int IODriverID;
}Cell;

Actor * newCell();
void deleteCell(Actor *);

#endif