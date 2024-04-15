#ifndef __CLIENT_THREADS_H__
#define __CLIENT_THREADS_H__

typedef struct ClientThreadsParams_s{
	size_t myrank;
	char* ServerName;
	int   portNum;
	char* Line;
}ClientThreadsParams;

void ClientThreadsParams_set(ClientThreadsParams* CTP ,size_t rank , char* ServerName,int ServerPort, char* Line);


void* client_thread_func(void* ClientParametres);







#endif
