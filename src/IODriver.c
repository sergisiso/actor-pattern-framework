#include "IODriver.h"



void initIODriver(Actor * self){
	IODriver * selfd = (IODriver *) self->derived;
	int *data = (int *)self->additional_msg_buffer;

	selfd->cellsInfo = (int *)malloc(data[0] * 2 * sizeof(int));
	selfd->num_cells = data[0];
	selfd->counter = 0;
	self->additional_buffer_size = 3;
	self->additional_msg_buffer = (int *) malloc(3*sizeof(int));
}


void receiveCellInfo(Actor * self){

	IODriver * selfd = (IODriver *) self->derived;
	int *data = (int *)self->additional_msg_buffer;
	int i;

    //Receive the cell population influx and infection level    
	selfd->cellsInfo[2*data[0]] = data[1];
	selfd->cellsInfo[(2*data[0])+1] = data[2];
	selfd->counter++;

    //When all the cells have sended their values, write it on the file
	if(selfd->counter == selfd->num_cells){

		selfd->cells_file = fopen("cell_values_per_year.txt", "a");
		if (selfd->cells_file == NULL){
			printf("ERROR: Could not open the cell output file\n");
			exit(-1);
		}

		fprintf(selfd->cells_file, "Year : %d\n", selfd->year);
		for(i=0; i < selfd->num_cells; i++){
			fprintf(selfd->cells_file, "Cell %d: populationInflux = %d, infectionLevel = %d\n",
				    i,selfd->cellsInfo[2*i],selfd->cellsInfo[(2*i)+1]);
		}
		fclose(selfd->cells_file);

        //Update counters
		selfd->year++;
		selfd->counter = 0;
	}
}



Actor * newIODriver(){

	IODriver * d = (IODriver *) malloc(sizeof(IODriver));
	d->base = newActor(4);
	d->base->derived = d;
	strcpy(d->base->actor_name,"IODriver");


	d->base->additional_buffer_size = 1;
	d->base->additional_msg_buffer = (int *) malloc(1*sizeof(int));
    d->year=1;
	d->cells_file = fopen("cell_values_per_year.txt", "w");
	if (d->cells_file == NULL){
		printf("ERROR: Could not open the cell output file\n");
		exit(-1);
	}
	fprintf(d->cells_file, "Simulation of frog population.\n");
	fclose(d->cells_file);
	
	d->base->del = &deleteIODriver;

	registerAction(d->base, INITIO_MSG, &initIODriver);
	registerAction(d->base, CELLVALS_MSG, &receiveCellInfo);


	return d->base;
}
void deleteIODriver(Actor * self){
	free(self);
}
