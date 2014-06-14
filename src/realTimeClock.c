#include "realTimeClock.h"



// Save the senderID, and execute the timer
void startTimer(Actor * self){
	realTimeClock * selfd = (realTimeClock *)self->derived;
	int data = self->additional_msg_buffer[0];

	selfd->clockID = data;
	sendMsg(self, getID(self), STOPT_MSG );
}

//When 100ms passed, send the trigger message to the saved ID
void stopTimer(Actor * self){
	realTimeClock * selfd = (realTimeClock *)self->derived;
	usleep(100000);
	sendMsg(self, selfd->clockID, TRIGGER_MSG );
}


Actor * newrealTimeClock(){
	realTimeClock * r = (realTimeClock *) malloc(sizeof(realTimeClock));
	r->base = newActor(4);
	r->base->derived = r;
	strcpy(r->base->actor_name,"rtClock");


	r->base->additional_buffer_size = 1;
	r->base->additional_msg_buffer = (int *) malloc(1*sizeof(int));

	registerAction(r->base, STARTT_MSG, &startTimer);
	registerAction(r->base, STOPT_MSG, &stopTimer);
	r->base->del = deleterealTimeClock;
	return r->base;
}
void deleterealTimeClock(Actor * self){

	free(self);
}
