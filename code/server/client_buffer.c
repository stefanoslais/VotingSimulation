#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

#include "client_buffer.h"

/*Each node of Buffer List  holds the client socket and a pointer to the next node*/
struct BufferNode_s {
	int ClientSocket;
	struct BufferNode_s* nextBufferNode;
};
typedef struct BufferNode_s BufferNode;

/*Some useful functions*/
bool ClientBuffer_isempty(ClientBuffer* CBptr) {	return CBptr->size == 0; 		}
bool ClientBuffer_isfull(ClientBuffer* CBptr)  {	return CBptr->size >= CBptr->MaxSize;	}
void ClientBuffer_initialize(ClientBuffer* CBptr , int maxsize) {
	CBptr->head = NULL;
	CBptr->tail = NULL;
	CBptr->size = 0;
	CBptr->MaxSize = maxsize;
	
	CBptr->Terminate = false;

	pthread_mutex_init(&(CBptr->CB_Lock), NULL);
	pthread_cond_init(&(CBptr->NoNEmpty_cond), NULL);
	pthread_cond_init(&(CBptr->NoNFull_cond), NULL);

}

/*Only one thread can write to the buffer as long as it not already full*/
void ClientBuffer_add(ClientBuffer *CBptr, int client_socket) {
/*Locking the structure*/
	pthread_mutex_lock(&(CBptr->CB_Lock));

/*If the structure is full, wait...*/
	while(ClientBuffer_isfull(CBptr)) {
		pthread_cond_wait(&(CBptr->NoNFull_cond),&(CBptr->CB_Lock));
	}

/*Now that we have escaped the waiting, NoNFull condition is 'true' and we have the structure to our own*/
/*Let's add the new node*/
/*create and initialize the new node*/
	BufferNode* newBN = malloc(sizeof(BufferNode));
	newBN -> ClientSocket = client_socket;
	newBN -> nextBufferNode = NULL;
/*add it to the end of the list*/
	if(ClientBuffer_isempty(CBptr)) {
		CBptr->head = newBN;
		CBptr->tail = newBN;
	}
	else
	{ 
		CBptr->tail->nextBufferNode = newBN;
		CBptr->tail = newBN;
	}
	(CBptr->size)++;
	
/*we no longer need the list, unlock the structure and notify other threads that the structure holds at least one element*/

	pthread_cond_broadcast(&(CBptr->NoNEmpty_cond));
	pthread_mutex_unlock(&(CBptr->CB_Lock));
}

/*Only one thread can grab an element from the list at a time, as long as it is not empty*/
int ClientBuffer_remove(ClientBuffer* CBptr) {

/*Locking the structure*/
	pthread_mutex_lock(&(CBptr->CB_Lock));

/*If the structure is empty,...*/
	while(ClientBuffer_isempty(CBptr)) {
/*and the reason it is empty is because Terminate flag is true*/
		if( CBptr->Terminate == true) {
/*return -1 for the thread to be terminated*/
			pthread_mutex_unlock(&(CBptr->CB_Lock));
			return -1;
		}
/*otherwise, wait for the list to not be empty*/
		pthread_cond_wait(&(CBptr->NoNEmpty_cond),&(CBptr->CB_Lock));
	}
/*Now that we have escaped the waiting, NoNEmpty condition is 'true' and we have the structure to our own*/
/*Let's obtain the Client Socket and remove the first node*/
	int client_socket = CBptr->head->ClientSocket;
	BufferNode* DeleteNode= CBptr->head;
/*new head will point to either the next item on the list or at NULL*/
	CBptr->head = DeleteNode->nextBufferNode;
	free(DeleteNode);
	(CBptr->size)--;
/*If there was just one elemenet before deletion, head and tail point to the same node. We need to take care of the tail*/
	if(ClientBuffer_isempty(CBptr))	CBptr->tail = NULL;

/*we no longer need the list, unlock the structure*/
	pthread_cond_broadcast(&(CBptr->NoNFull_cond));
	pthread_mutex_unlock(&(CBptr->CB_Lock));

	return client_socket;
}	

/*This function is called after SIGINT has been sent to the Master Thread. At that point, the list is certainly empty.
All threads are waiting on NoNEmpty condition , so by broadcasting one last time, we give them the opportunity to repeat their
while statement, thus checking the true value of Terminate flag. Upon seeing that Terminate == true, they exit*/
void ClientBuffer_close(ClientBuffer* CBptr, int NumThreads) {

	pthread_mutex_lock(&(CBptr->CB_Lock));
	CBptr->Terminate = true;
	pthread_mutex_unlock(&CBptr->CB_Lock);

	for(int ThreadWaiting = 0; ThreadWaiting < NumThreads; ThreadWaiting++)
		pthread_cond_broadcast(&(CBptr->NoNEmpty_cond));
	
}

void ClientBuffer_destroy(ClientBuffer* CBptr) {

/*Possibly uneccessary free but let's be safe*/
	BufferNode* next = CBptr->head;
	for(int node = 0; node < CBptr->size; node++ ) free(next);

	pthread_mutex_destroy(&(CBptr->CB_Lock));
	pthread_cond_destroy(&(CBptr->NoNEmpty_cond));
	pthread_cond_destroy(&(CBptr->NoNFull_cond));

	CBptr->head = NULL;
	CBptr->tail = NULL;
}

