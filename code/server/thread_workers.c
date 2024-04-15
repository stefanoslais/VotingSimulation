#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#include "thread_workers.h"
#include "client_buffer.h"
#include "../commonlib/message_peer.h"		/*for send_toPeer() */
#include "../commonlib/string_manipulation.h"	/*for parseLine() */
#include "poll_logging.h"		/*for MultipleThreadFile structure*/
#include "poll_statistics.h"		/*for PartiesList functions */

#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


#define MESS_SZ 100

extern bool ReceiveNewClients;

void* thread_f(void * voided_params) { 
/*Number 1 priority: signal handling*/
/*Set up mask to blovk SIGINT signal*/
	sigset_t WorkerThreadMask;
	sigemptyset(&WorkerThreadMask);
	sigaddset(&WorkerThreadMask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &WorkerThreadMask, NULL);
/*Finished signal handling*/

/*Extract the parametres*/
	struct WorkerThreadsParams* paramsptr = (struct WorkerThreadsParams*) voided_params;
	int myrank = paramsptr->rank;
	ClientBuffer* myCBptr = paramsptr->CBptr;
	char* PollLog = paramsptr->PollLog;
	PartiesList* myPLptr = paramsptr->PLptr;

/*Thread grabs a client socket. If it returns with -1, that means it's time for Thread Worker to exit*/
	int ClientSocket = ClientBuffer_remove(myCBptr);
	if( ClientSocket == -1 ) {
		printf("Thread %d: Exiting...\n",myrank);
		pthread_exit(NULL);
		return NULL;
	}

/*Server-Client exhange messages starts here*/
	char* MessageFromServer = malloc(MESS_SZ * sizeof(char));
	char* MessageFromClient = malloc(MESS_SZ * sizeof(char));
	initialize_MessageBuffer(MessageFromServer, MESS_SZ);	
	initialize_MessageBuffer(MessageFromClient, MESS_SZ);	

/*Server send "SEND NAME PLEASE"*/
	strcpy(MessageFromServer,"SEND NAME PLEASE");
	send_toPeer(ClientSocket, MessageFromServer, MESS_SZ);

/*Server receives name and surname*/
	recv(ClientSocket, MessageFromClient, MESS_SZ, MSG_WAITALL);
//printf("Thread %d: Client's name and surname is: %s\n", myrank, MessageFromClient);

/*Save Voter name and surname. The following two strings are dynamically allocated inside ParseNameSurname()*/
	char* VoterName = NULL;
	char* VoterSurname = NULL;
	parseLine(MessageFromClient, &VoterName, &VoterSurname , NULL, false);

/*The following structure is an abstraction of a file that handles multiple readers and writers*/
	MultiThreadFileRW PollLog_RW;	
	MultiThreadFileRW_initialize(&PollLog_RW,PollLog);
/*Tries to find the Voter with the corresponding name and surname*/
	if(FindVoter(&PollLog_RW, VoterName, VoterSurname) == false) {
/*If name doesn't exist. Server send "SEND VOTE PLEASE"*/
		initialize_MessageBuffer(MessageFromServer, MESS_SZ);
		strcpy(MessageFromServer,"SEND VOTE PLEASE");	
		
		send_toPeer(ClientSocket, MessageFromServer, MESS_SZ); 

/*Server receives party*/	
		initialize_MessageBuffer(MessageFromClient, MESS_SZ);
		recv(ClientSocket,MessageFromClient, MESS_SZ, MSG_WAITALL);

/*Save party. Party is one word*/
		char* VoterParty = NULL;
		VoterParty = malloc((strlen(MessageFromClient) + 1) * sizeof(char));
		strcpy(VoterParty, MessageFromClient);
//printf("Thread %d: %s %s voted for %s\n",myrank, VoterName, VoterSurname, VoterParty);

/*Insert Voter Information to Poll Log*/
		AppendVote(&PollLog_RW, VoterName, VoterSurname, VoterParty);	
/*Update the structure that holds the results of the election*/
		PartiesList_addVoteToParty(myPLptr, VoterParty);

/*Server send confirmation Message. MessageFromServer is definately null terminated*/
		initialize_MessageBuffer(MessageFromServer, MESS_SZ);
		sprintf(MessageFromServer, "VOTE for Party %s RECORDED", VoterParty);

		send_toPeer(ClientSocket, MessageFromServer, MESS_SZ);

		free(VoterParty);  
	}
	else 
	{
/*If name does exist*/
//printf("Thread %d: %s %s has already voted\n", myrank, VoterName,VoterSurname);
/*Server send "ALREADY VOTED". MessageFromServer is definately null terminated*/
		initialize_MessageBuffer(MessageFromServer, MESS_SZ);
		strcpy(MessageFromServer,"ALREADY VOTED");

		send_toPeer(ClientSocket, MessageFromServer, MESS_SZ);
	}

/*Server-Client exchanging messages ends here*/
	close(ClientSocket);
/*Freeing things this thread no longer needs*/
	free(VoterName); VoterName = NULL;
	free(VoterSurname); VoterSurname = NULL;

	free_MessageBuffer(&MessageFromServer);
	free_MessageBuffer(&MessageFromClient);

/*Same thread calls itself to continue with other clients*/
	thread_f(voided_params);
}
