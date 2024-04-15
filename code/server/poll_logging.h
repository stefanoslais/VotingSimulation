#ifndef __POLL_LOGGING_H__
#define __POLL_LOGGING_H__

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
typedef struct MultiThreadFileRW_s{
	char* FileName;
	pthread_mutex_t FileAccess_lock;
	pthread_cond_t ThreadCanWrite_cond;
	pthread_cond_t ThreadCanRead_cond;
	unsigned int ActiveWriters;
	unsigned int WaitingWriters;
	unsigned int ActiveReaders;
	unsigned int WaitingReaders;

}MultiThreadFileRW;

void MultiThreadFileRW_initialize(MultiThreadFileRW* MTFileRW,char* fileptr);
bool FindVoter(MultiThreadFileRW* File, char* VoterName, char* VoterSurname);
void AppendVote(MultiThreadFileRW* File,char* VoterName, char* VoterSurname, char* VoterParty);

#endif
