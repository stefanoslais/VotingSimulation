#ifndef __POLL_STATISTICS_H__
#define __POLL_STATISTICS_H__

#include <pthread.h>
struct PartyNode_s{
	char* PartyName;
	int   PartyNumber;		/*the number of its votes*/
	struct PartyNode_s* nextPartyNode;
};
typedef struct PartyNode_s PartyNode;

struct PartiesList_s{
	PartyNode* head;
	PartyNode* tail;
	int NumberofParties;		/*total number of parties*/
	pthread_mutex_t List_Lock;
};
typedef struct PartiesList_s PartiesList;

void PartiesList_initialize(PartiesList* PNL);
void PartiesList_addVoteToParty(PartiesList* PNL , char* PartyName);
void PartiesList_appendtoFile(PartiesList* PNL, char* filename);
void PartiesList_destroy(PartiesList* PNL);


#endif
