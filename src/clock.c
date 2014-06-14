#include "clock.h"


void startSimulationClock(Actor * self){
	Clock * selfd = (Clock *)self->derived;
	int * parameters = (int *)self->additional_msg_buffer;
	int i, newID;


    // I do not use the initial sended initial parameters because
    // in Morar system they are not the expected, in my laptop that
    // works
	selfd->ini_frogs = 32;// parameters[0];
	selfd->max_frogs = 100;// parameters[1];
	selfd->number_cells = 16;//parameters[2];
	selfd->ini_inf_level = 4;// parameters[3];
	selfd->num_years = 200;// parameters[4];
	selfd->current_year = 1;


    printf("Starting frog disease simulation with parameters:\n");
    printf(" - Initial frogs           : %d\n",selfd->ini_frogs);
    printf(" - Maximum frogs           : %d\n",selfd->max_frogs);
    printf(" - Number Cells            : %d\n",selfd->number_cells);
    printf(" - Initial Infection Level : %d\n",selfd->ini_inf_level);
    printf(" - Number of years         : %d\n",selfd->num_years);


	selfd->frogs_io_counter = 0;
	selfd->year_time_counter = 0;

	self->additional_buffer_size = 1;
	self->additional_msg_buffer = (int *) malloc(1*sizeof(int));


	int frog_data[selfd->number_cells + 2];
	int driver_data[1];
	int cell_data[2];
	

	// Initialize frog and cell lists
	selfd->cellID = (int *)calloc(selfd->number_cells, sizeof(int));
	selfd->frogID = (int *)calloc(selfd->max_frogs , sizeof(int));
	selfd->current_frogs = selfd->ini_frogs;
	

	// Create all the starting Actors
	printf("[Simulation] Create IODriver\n" );
	selfd->IODriverID = spawn(self, "IODriver");
	driver_data[0] = selfd->number_cells;
	sendMsgData(self, selfd->IODriverID, INITIO_MSG, (void *) &driver_data, 1);


	printf("[Simulation] Create %d cells\n", selfd->number_cells );
	cell_data[0] = selfd->IODriverID;
	for(i=0; i < selfd->number_cells; i++){
		newID = spawn(self, "cell");
		cell_data[1]= i;
		sendMsgData(self, newID, CELLINIT_MSG, (void *) &cell_data, 2);
		selfd->cellID[i] = newID;
		frog_data[i+2] = newID;
	}


	printf("[Simulation] Create %d sane frogs\n", selfd->ini_frogs - selfd->ini_inf_level );
	frog_data[0] = 0;
	frog_data[1] = getID(self);
	for(i=0; i < selfd->ini_frogs - selfd->ini_inf_level; i++){
		newID = spawn(self, "frog");
		sendMsgData(self, newID, FROGINIT_MSG, (void *) &frog_data, selfd->number_cells+2);
		selfd->frogID[i] = newID;

	}

	printf("[Simulation] Create %d infected frogs\n", selfd->ini_inf_level );
	frog_data[0] = 1;
	for(; i < selfd->ini_frogs; i++){
		newID = spawn(self, "frog");
		sendMsgData(self, newID, FROGINIT_MSG, (void *) &frog_data, selfd->number_cells+2);
		selfd->frogID[i] = newID;
	}
	
	selfd->rtClockID = spawn(self, "rtClock");
	int id = getID(self);
	sendMsgData(self, selfd->rtClockID, STARTT_MSG, (void *)&id,1);

}

void terminateSimulation(Actor * self){
	Clock * selfd = (Clock *)self->derived;
	int i;

	// Send all the necessary kill messages
	printf("[Simulation] Terminate Simulation!.\n");
    for(i=0; i < selfd->number_cells; i++) sendMsg(self, selfd->cellID[i], KILL_MSG);
    for(i=0; i < selfd->max_frogs; i++){
    	if( selfd->frogID[i] != 0) sendMsg(self, selfd->frogID[i], KILL_MSG);
    }   
    sendMsg(self,selfd->IODriverID,KILL_MSG);
    sendMsg(self,selfd->rtClockID,KILL_MSG);
    kill(self);
	terminate(self);
}


// Executed every 100ms 
void clockTrigger(Actor * self){
	Clock * selfd = (Clock *)self->derived;
	int i;

	selfd->frogs_io_counter++;
	selfd->year_time_counter++;

	// Every one second print the number of frogs
	if( selfd->frogs_io_counter == 10){
		printf("Number of frogs = %d\n", selfd->current_frogs);
		selfd->frogs_io_counter = 0;
	}
	
	// Every 1 seconds new year
	if (selfd->year_time_counter == 10){
		printf("[Simulation] -------------- Year %d -------------- \n",selfd->current_year);
		selfd->current_year++;


		if (selfd->current_year > selfd->num_years){
			// End simulation
			for(i=0; i<selfd->number_cells; i++) sendMsg(self, selfd->cellID[i], MONSOON_MSG);
			terminateSimulation(self);
			return;
		}else{
			// Monsoon and continue simulation
			for(i=0; i<selfd->number_cells; i++) sendMsg(self, selfd->cellID[i], MONSOON_MSG);
		}
		selfd->year_time_counter=0;
	}


	int id = getID(self);
	sendMsgData(self, selfd->rtClockID, STARTT_MSG, (void *)&id,1);
}



// Called when there is a new frog, in order to keep track of it
void birthFrog(Actor * self){
	Clock * selfd = (Clock *)self->derived;
	int frog_data[selfd->number_cells+2];
	int i;
	int newid = self->additional_msg_buffer[0];


	selfd->frogID[selfd->current_frogs] = newid;
	selfd->current_frogs++;
	frog_data[0] = 0;
	frog_data[1] = getID(self);
	for(i=0; i<selfd->number_cells; i++) frog_data[i+2] = selfd->cellID[i];
	sendMsgData(self, newid, FROGINIT_MSG, (void *) &frog_data, selfd->number_cells + 2 );
		
    if (selfd->current_frogs >= selfd->max_frogs ){
		printf("ERROR: Maximum number of frogs exceeded!\n");    
		terminateSimulation(self);
		return;
	}

	if(SIM_DEBUG_MORE) printf("new frog id: %d\n",newid );
}

//Called when a frog dies, in order to remove it from the list
void deathFrog(Actor * self){
	Clock * selfd = (Clock *)self->derived;
	int oldid = self->additional_msg_buffer[0];
	int i;

	if(SIM_DEBUG_MORE) printf("Death Frog id: %d\n", oldid);

	if (selfd->current_frogs - 1 <= 0){
		printf("ERROR: All frogs are death!\n");
		terminateSimulation(self);
		return;
	}else{
		// Search the frog id in the list
		i = 0;
		while(selfd->frogID[i] != oldid && selfd->frogID[i] != 0)i++;
		if(selfd->frogID[i] == 0){
			printf("ERROR: death frog not in the list\n");
			terminateSimulation(self);
			return;
		}
		if(selfd->frogID[i] == oldid){
			// Move the last id to this possition and put a 0 in the last one
			selfd->current_frogs--;
			selfd->frogID[i] = selfd->frogID[selfd->current_frogs];
			selfd->frogID[selfd->current_frogs] = 0;
        }
	}
}


Actor * newClock(){
	Clock * c = (Clock *) malloc(sizeof(Clock));
	c->base = newActor(10);
	c->base->derived = c;
	strcpy(c->base->actor_name,"clock");

	c->base->additional_buffer_size = 5;
	
    c->base->additional_msg_buffer = (int *) malloc(5*sizeof(int));
    c->base->additional_msg_buffer[0] = 1000;

	registerAction(c->base, START_MSG, &startSimulationClock);
	registerAction(c->base, NEW_MSG, &birthFrog);
	registerAction(c->base, DIE_MSG, &deathFrog);
	registerAction(c->base, TRIGGER_MSG, &clockTrigger);


	c->base->del = deleteClock;
	return c->base;
}

void deleteClock(Actor * self){
	printf("Deleting clock\n");
	free(self);
}
