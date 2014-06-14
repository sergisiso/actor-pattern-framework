#include <stdio.h>


//#include "frogSimulation.h"
#include "actorPatternFramework.h"
#include "mpi.h"
#include "clock.h"
#include "cell.h"
#include "frog.h"
#include "realTimeClock.h"
#include "IODriver.h"



int main(int argc, char** argv){
  	int ini_frogs = 32;
  	int max_frogs = 100;
 	int number_cells = 16;
 	int ini_inf_level = 10;
	int num_years = 10;

    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /*if(rank==0){
        printf("Starting frog disease simulation with parameters:\n");
        printf(" - Initial frogs           : %d\n",ini_frogs);
        printf(" - Maximum frogs           : %d\n",max_frogs);
        printf(" - Number Cells            : %d\n",number_cells);
        printf(" - Initial Infection Level : %d\n",ini_inf_level);
        printf(" - Number of years         : %d\n",num_years);
    }*/

    int parameters[5] = {ini_frogs, max_frogs, number_cells, ini_inf_level, num_years};

    initializeActorPatternController(5, MPI_COMM_WORLD, 100);
    registerActorType("clock", &newClock);
    registerActorType("cell", &newCell);
    registerActorType("frog", &newFrog);
    registerActorType("IODriver", &newIODriver);
    registerActorType("rtClock", &newrealTimeClock);


    double t1 = MPI_Wtime();
    executeAP("clock",START_MSG, parameters, 5);
    double t2 = MPI_Wtime();

    if(rank==0){
        printf("\nSimulation run successfully!\n");
        printf("Simulation time: %f s\n", t2-t1);
    }

    finalizeActorPatternController();

    MPI_Finalize();
    return 0;
}
