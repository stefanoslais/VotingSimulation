#include "poll_statistics.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

void PartiesList_initialize( PartiesList* PNL)	{
	PNL->tail = NULL;
	PNL->head = NULL;
	PNL->NumberofParties = 0;
	pthread_mutex_init(&(PNL->List_Lock), NULL);
}

PartyNode* PartyNode_create(char* PartyName) {
	PartyNode* newPartyN = malloc(sizeof(PartyNode));
	newPartyN->PartyName = malloc(strlen(PartyName) + 1);
	strcpy(newPartyN->PartyName,PartyName);
	newPartyN->PartyNumber = 1;
	newPartyN->nextPartyNode = NULL;

	return newPartyN;
}



bool PartiesList_isempty( PartiesList* PL )	{	return (PL->NumberofParties == 0);	}

/*Only one thread that has something to add to the structure can enter*/
void PartiesList_addVoteToParty(PartiesList * PNL , char* PartyName) {
	pthread_mutex_lock(&(PNL->List_Lock));
/*If the Party List is empty, create the node and add it to the list*/
	if(PartiesList_isempty(PNL)) {
		PartyNode* PN = PartyNode_create(PartyName);
		PNL->head = PN;
		PNL->tail = PN;
		PNL->NumberofParties = 1;
		pthread_mutex_unlock(&(PNL->List_Lock));
 		return;
	}
/*If the list is not empty, search each node for the party*/
	PartyNode* nextNode = PNL->head;
	int NodeIndex = 0;
	while( NodeIndex < PNL->NumberofParties  && strcmp(nextNode->PartyName, PartyName) != 0 ) {
		nextNode = nextNode -> nextPartyNode;
		NodeIndex++;
	}
/*If it hasn't found this Party, add it at the end of the list*/
	if(NodeIndex == PNL->NumberofParties) {
		PartyNode* PN = PartyNode_create(PartyName);
		
		PNL->tail->nextPartyNode = PN;
		PNL->tail = PN;
		(PNL->NumberofParties)++;
	}
/*If it has found the party, just add the PartyNumber by one*/
	else	(nextNode->PartyNumber)++;

	pthread_mutex_unlock(&(PNL->List_Lock));
	
	return ;
}	

void PartiesList_destroy(PartiesList* PNL) { 
	PartyNode* next = PNL->head;
	for(int party = 0; party < PNL->NumberofParties; party++) {
		PartyNode* deleted = next;
		next = deleted->nextPartyNode;
		free(deleted->PartyName);
		free(deleted);
	}
	pthread_mutex_destroy(&(PNL->List_Lock));
	return ;
}

/*function that is called in Master Thread and copies information from the Parties List to the poll stats file*/
void PartiesList_appendtoFile(PartiesList* PNL, char* filename){

	FILE* pollStats;
	if( (pollStats = fopen(filename,"a")) == NULL) {
		perror("poll stats opening");
		exit(EXIT_FAILURE);
	}
	int totalVotes = 0;	
	PartyNode* node = PNL->head;
	for( int thisParty = 0; thisParty < PNL->NumberofParties; thisParty++) {
		fprintf(pollStats,"%s %d\n",node->PartyName,node->PartyNumber);
		totalVotes += node->PartyNumber;
		node = node->nextPartyNode;
	}

	fprintf(pollStats,"TOTAL %d\n",totalVotes);
	fclose(pollStats);
	return;
}
	





