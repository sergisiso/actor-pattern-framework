#include "frog.h"
#include "unistd.h"


void parameters(Actor * self){
	Frog * selfd = (Frog *) self->derived;
	int i;
	int *data = (int *)self->additional_msg_buffer;

    //receive the parameters
	selfd->infected = data[0];
	selfd->clockId = data[1];
	for(i=0; i<16; i++){
		selfd->cellIDs[i] = data[i+2];
	}

	self->additional_buffer_size = 2;
	self->additional_msg_buffer = (int *) malloc(2*sizeof(int));

    //start hopping
	sendMsg(self, getID(self), HOP_MSG);

}

void makeHop(Actor * self){

	Frog * selfd = (Frog *)self->derived; //Polymorphic transformation
	float newx, newy;

    // In a random time, jump again
    int r = rand() % 2000;
    usleep(r);

	//Update frog
	frogHop(selfd->x, selfd->y, &newx, &newy, &(selfd->state));
	selfd->x = newx;
	selfd->y = newy;
	if(getCellFromPosition(newx, newy)>=16){
		printf("Cell out of index\n");
		exit(0);
	}


	//Update Cell
	int newCellId = selfd->cellIDs[getCellFromPosition(newx, newy)];
	if(SIM_DEBUG_MORE) printf("Frog hops to cell %d\n",getCellFromPosition(newx, newy));
	sendMsgData(self, newCellId, HOPIN_MSG, (void *) &(selfd->infected), 1);

}

void receiveEnvironmentalFacotrs(Actor * self){
	Frog * selfd = (Frog *)self->derived;
	int * data = (int *)self->additional_msg_buffer;

	// Every 300 hops chance to give birth
    //  * I have added a factor of 1.2 because the population
    //    quickly exceeds the maximum number of frogs !!!

	selfd->hop_counter_birth++;
	selfd->sum_pop_influx += data[0];
	if(selfd->hop_counter_birth == 300 ){
		if(willGiveBirth((float)selfd->sum_pop_influx/300, &(selfd->state)) == 1){
			if(SIM_DEBUG_FEW) printf(" -> Frog %d give birth\n",getID(self));
			int newid = spawn(self, "frog");
			sendMsgData(self, selfd->clockId, NEW_MSG,(void *) &newid, 1);
		}
		selfd->sum_pop_influx = 0;
		selfd->hop_counter_birth = 0;
	}


    selfd->hop_counter_death++;	
	if(selfd->infected){
		// If infected , Every 700 hops chance to die
		if(selfd->hop_counter_death == 700){
			if(willDie(&(selfd->state))){
				if(SIM_DEBUG_FEW) printf(" -> Frog %d dies.\n", getID(self));
				int myid = getID(self);
				sendMsgData(self, selfd->clockId, DIE_MSG,(void *) &myid, 1);
				kill(self);
				return;
			}
			selfd->hop_counter_death = 0;
		}
	}else{
		// Not infected, change to catch disease
		selfd->hop_counter_catch++;
		if (selfd->hop_counter_catch == 500) selfd->hop_counter_catch = 0;
		selfd->infection_sum -= selfd->infection_history[selfd->hop_counter_catch];
		selfd->infection_history[selfd->hop_counter_catch] = data[1];
		selfd->infection_sum += selfd->infection_history[selfd->hop_counter_catch];
		if( willCatchDisease((float)selfd->infection_sum/500, &(selfd->state))){
			if(SIM_DEBUG_FEW) printf("-> Frog %d infected\n",getID(self));
			selfd->infected = 1;
		}	
	}

	// If it is alive, hop again
	sendMsg(self, getID(self), HOP_MSG);
} 


Actor * newFrog(){

	Frog * f = (Frog *) malloc(sizeof(Frog));
	f->base = newActor(8);
	f->base->derived = f;
	strcpy(f->base->actor_name, "frog");


	f->base->additional_buffer_size = 18;
	f->base->additional_msg_buffer = (int *) malloc(18*sizeof(int));

	// Initialize frog counters
	long seed = -1 -(long)getID(f->base);
	initialiseRNG(&seed);
	f->state = seed;
	f->hop_counter_birth = 0;
	f->hop_counter_death = 0;
	f->hop_counter_catch = 0;
	f->sum_pop_influx = 0;
	f->infection_sum = 0;
	f->infection_history = (int *) calloc(500, sizeof(int));

	//Initial position
	frogHop(0, 0, &(f->x), &(f->y), &(f->state));


	registerAction(f->base, FROGINIT_MSG, &parameters);
	registerAction(f->base, HOP_MSG, &makeHop);
	registerAction(f->base, ENV_FACT_MSG, &receiveEnvironmentalFacotrs);
	return f->base;
}

void deleteFrog(Frog * self){
	free(self->infection_history);
	free(self);
}
