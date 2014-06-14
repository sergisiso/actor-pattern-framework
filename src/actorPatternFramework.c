#include "actorPatternFramework.h"
#include <unistd.h>



// Definition of ActorPatternController Private methods
int matchActor(const char * type_name);
void registerActorType(const char * name, ptrToActorCreator pac);
void masterCode(const char * starting_actor,  int starting_msg_id, int * starting_parameters, int num_parameters);
void workerCode();


// Definition of Actor Private methods
ptrToAction matchAction(Actor * self, int msg_id);


//Implementation of ActorPatternController methods
void initializeActorPatternController(int num_actors, MPI_Comm comm, int buffer_size){
 	int i;
	int buffsize = buffer_size * (sizeof(int)+MPI_BSEND_OVERHEAD);

    AP = (ActorPatternController *)malloc(sizeof(ActorPatternController));

	// Allocate data structures
	AP->communicatior = comm;
    MPI_Comm_size(comm, &(AP->commsize));
    MPI_Comm_rank(comm, &(AP->rank));

 	AP->actor_t_list = (struct actor_type_node *) malloc(num_actors * sizeof(struct actor_type_node));
 	MPI_Buffer_attach(malloc(buffsize),buffsize );

 	AP->num_actors = num_actors;
 	for(i=0; i < num_actors; i++){
 		AP->actor_t_list[i].name = NULL;
 		AP->actor_t_list[i].actor_creator = NULL;
 	}

}

void registerActorType(const char * name, ptrToActorCreator pac){
	int i = 0;
	while(AP->actor_t_list[i].name != NULL)i++;
	AP->actor_t_list[i].name = name;
	AP->actor_t_list[i].actor_creator = pac;
	if(AP_DEBUG) printf("Actor type %s registred in possition %d!\n", name, i);
}


int matchActor(const char * type_name){
	int i;
	for(i=0; i < AP->num_actors; i++){
		if(strcmp(AP->actor_t_list[i].name,type_name) == 0) return i;
	}
	printf("Error: Any match for actor: %s\n", type_name );
	return -1;
}

void masterCode(const char * starting_actor,  int starting_msg_id,
				int * starting_parameters, int num_parameters){

	int activeWorkers=0;

	int workerPid = startWorkerProcess();
	int type = matchActor(starting_actor);
	int actorID;

	activeWorkers++;

	// Create a strarting_actor Actor
	MPI_Send(&type, 1, MPI_INT, workerPid, 2, AP->communicatior);
	MPI_Recv(&actorID, 1, MPI_INT , workerPid, 3, AP->communicatior, MPI_STATUS_IGNORE);

	// Send to it the initial message and the initial parameters.
	if(num_parameters == 0){
		message msg = {-1, actorID, starting_msg_id, 0};
		MPI_Bsend(&msg, 4, MPI_INT, workerPid, 10, AP->communicatior);
	}else if(num_parameters > 0){
		message msg = {-1, actorID, starting_msg_id, 1};
		MPI_Bsend(&msg, 4, MPI_INT, workerPid, 10, AP->communicatior);
		MPI_Bsend(starting_parameters, num_parameters, MPI_INT, workerPid, 20, AP->communicatior);
	}else{
		printf("Error: Number of starting parameters should be >= 0\n");
		MPI_Abort(AP->communicatior, -2);
	}


	if(AP_DEBUG)printf("Master started worker %s on MPI process %d with message %d\n",
	        AP->actor_t_list[type].name, workerPid, starting_msg_id);
	
	//Wait until the shut-down
	int masterStatus = masterPoll();
	while (masterStatus) {
		masterStatus=masterPoll();
	}
}

void workerCode(){
	int workerStatus = 1;
	int actor_counter = 0;
	MPI_Request r;
	MPI_Status s;
	int type;

	// while shut-down is not triggered do work
	while (workerStatus) {
		
		int creatorPID = getCommandData();
		
		// Recieve new Actor type and create the specified new Actor
		MPI_Recv(&type, 1, MPI_INT, creatorPID, 2, AP->communicatior, MPI_STATUS_IGNORE);
		actor_counter++;
		ptrToActorCreator ActorCreator = AP->actor_t_list[type].actor_creator;
		Actor * a = ActorCreator();

		// Assign and send back a new ID
		a->ID = (actor_counter * AP->commsize) + AP->rank;
		MPI_Send(&a->ID, 1, MPI_INT, creatorPID, 3, AP->communicatior);


		// while the actor is alive, receive messages and execute their actions
		while(!a->kill_actor){

			//Receive message
			message msg;
			MPI_Irecv(&msg, 4, MPI_INT, MPI_ANY_SOURCE, 10, AP->communicatior, &r);
			MPI_Wait(&r, &s);

			// Ignore message if it has not the proper ID (probably comes to an death Actor)
			if(msg[1] != getID(a)){
				if (msg[3] == 1){
					MPI_Irecv(a->additional_msg_buffer, a->additional_buffer_size,
									MPI_INT, msg[0]%AP->commsize, 20, AP->communicatior, &r);
					MPI_Wait(&r, MPI_STATUS_IGNORE);
				}
				continue;
			} 

			// If it is necessary, wait for more data from the same sender
			a->last_message_ID = msg[0];	
			if(msg[3] == 1){
				MPI_Irecv(a->additional_msg_buffer, a->additional_buffer_size, MPI_INT,
                        a->last_message_ID%AP->commsize, 20, AP->communicatior, &r);
				MPI_Wait(&r, &s);
			}


			if(AP_DEBUG) printf("[Actor %d] Received message with id %d\n", getID(a), msg[1]);

			// Execute the action
			ptrToAction actionToPerform = matchAction(a, msg[2]);
			if(actionToPerform != NULL){
				actionToPerform(a);
			}else{
				printf("[Actor %s] ERROR: msg id %d does not match with any function\n", a->actor_name, msg[2]);
			}
		}
		deleteActor(a);
		workerStatus = workerSleep();	// This MPI process will sleep, further workers may be run on this process now
	}
}

void executeAP(const char * starting_actor, int  starting_msg_id, int * starting_parameters, int num_parameters ){
    
	// Ensure that all processes have finished the initialization
    MPI_Barrier(MPI_COMM_WORLD);
    
    int statusCode = processPoolInit();
	if (statusCode == 2) {
		masterCode(starting_actor, starting_msg_id,starting_parameters,num_parameters);
	} else if (statusCode == 1) {
		workerCode();
	}
	processPoolFinalise();	
	
}

void finalizeActorPatternController(){
	int buffsize;
	char * buffer;
	MPI_Buffer_detach(&buffer, &buffsize);
	free(AP->actor_t_list);
	MPI_Barrier(MPI_COMM_WORLD);
}


// IMPLEMENTATION of the Actor class methods

Actor * newActor(int list_lenght){

	// Create new actor instance
	Actor * a = (Actor *) malloc(sizeof(Actor));

	//Allocate msg_id and action id arrays
	a->msg_id = (int *) malloc(list_lenght * sizeof(int));
	a->msg_action = (ptrToAction *) malloc(list_lenght * sizeof(ptrToAction));
	a->num_registred_actions = -1;
	a->max_actions = list_lenght + 1; //user functions + kill
	a->kill_actor = 0;


	// All actors have the kill action registered.
	// kill function has ptrToAction interface.
	// This must be called after initialize object attributes.
	registerAction(a, KILL_MSG, &kill);
	return a;
}

void deleteActor(Actor * self){
	//ptrToAction delfunc = self->del;
	//if(delfunc != NULL) delfunc(self);
	free(self->msg_action);
	free(self->msg_id);
	//free(self->derived);
	free(self);
}

void registerAction(Actor * self, int id, ptrToAction action){

	int next_action = self->num_registred_actions + 1;
	if(next_action < self->max_actions){
		self->msg_id[next_action] = id;
		self->msg_action[next_action] = action;
		self->num_registred_actions = next_action;
	}else{
		printf("[Actor %s] ERROR: Max actions exceeded.\n", self->actor_name);
	}

}

ptrToAction matchAction(Actor * self, int msg_id){
	int i;
	for(i=0;i<self->max_actions;i++){
		if(self->msg_id[i] == msg_id) return self->msg_action[i];
	}
	printf("[Actor %s] ERROR: Any match for the message: %d\n",self->actor_name,msg_id);
	return NULL;
}


int spawn(Actor * self, const char * type){
	int newID;
	int spawnedPid = startWorkerProcess();
	int index = matchActor(type);
	MPI_Send(&index, 1, MPI_INT, spawnedPid, 2, AP->communicatior);
	MPI_Recv(&newID, 1, MPI_INT, spawnedPid, 3, AP->communicatior, MPI_STATUS_IGNORE);
	return newID;
}
int getID(Actor * self){
	return self->ID;
}

void kill(Actor * self){
	self->kill_actor = 1;
}

void terminate(Actor * self){
	kill(self);
	shutdownPool();
}

void sendMsg(Actor * self, int toID, int msg_id){
	if(AP_DEBUG) printf("Sending %d to process %d\n", msg_id, toID );
	message msg = {getID(self),toID,msg_id,0};
	MPI_Bsend(&msg, 4, MPI_INT, toID%AP->commsize , 10, AP->communicatior);
}

void sendMsgData(Actor * self, int toID, int msg_id, void * additional_data, int additional_data_size){
	if(AP_DEBUG) printf("Sending %d to process %d\n", msg_id, toID );
	message msg = {getID(self),toID,msg_id,1};
	MPI_Bsend(&msg, 4, MPI_INT, toID%AP->commsize , 10, AP->communicatior);
    MPI_Bsend(additional_data, additional_data_size, MPI_INT, toID%AP->commsize, 20, AP->communicatior);
}
