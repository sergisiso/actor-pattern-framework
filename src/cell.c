#include "cell.h"


void frog_jumps_in(Actor * self){

	Cell * selfd = (Cell *)self->derived;
	int data = self->additional_msg_buffer[0];


	selfd->population_influx++;
	if(data == 1 ){ // if the frog is infected increase infection level 
		selfd->infection_level++;
	}

	//reply with the population_infux and the infection_level.
	int ctf[2];// = {selfd->population_influx, selfd->infection_level};
	ctf[0] = selfd->population_influx;
	ctf[1] = selfd->infection_level;
	sendMsgData(self, self->last_message_ID, ENV_FACT_MSG, (void *)&ctf, 2);
}

void monsoon(Actor * self){

	Cell * selfd = (Cell *)self->derived; 
	int data[3] = {selfd->CellNum,selfd->population_influx, selfd->infection_level };

    // Before zeroing the values, send them to the IODriver
	sendMsgData(self, selfd->IODriverID , CELLVALS_MSG, (void *)&data, 3);

	selfd->infection_level = 0;
	selfd->population_influx = 0;
}

void initCell(Actor * self){
	Cell * selfd = (Cell *) self->derived;
	int *data = (int *)self->additional_msg_buffer;

	selfd->IODriverID = data[0];
	selfd->CellNum = data[1];
}

Actor * newCell(){
	Cell * c = (Cell *)malloc(sizeof(Cell));
	c->base = newActor(5);
	c->base->derived = c;
	strcpy(c->base->actor_name, "cell");

	c->infection_level = 0;
	c->population_influx = 0;

	c->base->additional_buffer_size = 2;
	c->base->additional_msg_buffer = (int *) malloc(2*sizeof(int));

	registerAction(c->base, MONSOON_MSG, &monsoon);
	registerAction(c->base, HOPIN_MSG, &frog_jumps_in);
	registerAction(c->base, CELLINIT_MSG, &initCell);

	c->base->del = &deleteCell;
	return c->base;
}

void deleteCell(Actor * self){
	free(self);
}
