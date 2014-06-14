#ifndef ACTOR_H_
#define ACTOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include "pool.h"

#define AP_DEBUG 0
#define KILL_MSG 0


typedef int message[4];
// 0 -> fromID
// 1 -> toID
// 2 -> msg_id
// 3 -> has additional data?  


// Definiton of Actor
typedef struct Actor_ Actor;

// Define function pointers to Actor Action and Actor Creator functions.
typedef void (*ptrToAction)(Actor *); 
typedef Actor * (*ptrToActorCreator)();


// Node of the list of ActorNames and ActorCreator functions
struct actor_type_node
{
	const char * name;
	ptrToActorCreator actor_creator;
};


//
//	ActorPatternController class definition
//
typedef struct ActorPatternController{
	struct actor_type_node * actor_t_list;
	int num_actors;
	MPI_Comm communicatior;
	int commsize;
	int rank;

}ActorPatternController;
ActorPatternController * AP;
//
//	ActorPatternController class public methods
//

//Creates an ActorPatternController with space for num_actors types of Actors
void initializeActorPatternController(int num_actors, MPI_Comm comm, int buffer_size);
//Register as an Actor any subclass of Actor
void registerActorType( const char * name, ptrToActorCreator pac);
//Start execution: Indicate which actor start and with which message and parameters
void executeAP( const char * starting_actor, int starting_msg_id, int * starting_parameters, int num_parameters);
// Finalize the actorPatternController
void finalizeActorPatternController();


//
//	Actor class definition: All the actors must be a subclass of this one.
//
struct Actor_{
	void * derived; // ptr to subclass -> allows C polymorphism

	int ID; 

	char actor_name[10];
	int last_message_ID;
	int num_registred_actions;
	int max_actions;
	int kill_actor;
	int * additional_msg_buffer;
	int additional_buffer_size;
	ptrToAction del;

	//Lists of msgs <-> actions (function pointer)
	int * msg_id;
	ptrToAction * msg_action;
};

//Actor public functions

// Create the actor with list_lenght space to register Actor actions
Actor * newActor(int list_lenght);
// Delete the actor
void deleteActor(Actor * self);
// Return the ID of the actor
int getID(Actor * self);
// Register one action that will be executed when a certain id is received
// All actors are inizialised with the kill action with the id 0
void registerAction(Actor * self, int id, ptrToAction action);
//Spawn a new actor specifying his name
int spawn(Actor * self, const char * type);
// Send a simple messge to another actor
void sendMsg(Actor * self, int toID, int msg);
//Send a Message containing additional data to another actor
void sendMsgData(Actor * self, int toID, int msg, void * additional_data, int additional_data_size);
//Kill the actor
void kill(Actor * self);
// Triggers the stopping process, must be executed after sending a kill message to all the other actors.
void terminate(Actor * self);



#endif
