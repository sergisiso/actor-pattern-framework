#ifndef RTCLOCK_H_
#define RTCLOCK_H_

#include "actorPatternFramework.h"
#include "messages.h"
#include <time.h>
#include "unistd.h"

typedef struct realTimeClock{

	Actor * base;
	double last;
	int clockID;
}realTimeClock;


Actor * newrealTimeClock();
void deleterealTimeClock(Actor *);
#endif