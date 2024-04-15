#ifndef __CLIENT_BUFFER_H__
#define __CLIENT_BUFFER_H__

#include <stdbool.h>
#include <pthread.h>

struct BufferNode_s;
/*a queue with max size equal to buffer size given by the user*/
struct ClientBuffer_s {
	struct BufferNode_s* head;	
	struct BufferNode_s* tail;
	int size;
	int MaxSize;

	pthread_mutex_t CB_Lock;
	pthread_cond_t NoNEmpty_cond;
	pthread_cond_t NoNFull_cond;

	bool Terminate;
};
typedef struct ClientBuffer_s ClientBuffer;

void ClientBuffer_initialize(ClientBuffer *, int );
void ClientBuffer_close(ClientBuffer* , int numThreads);
void ClientBuffer_destroy(ClientBuffer*);

void ClientBuffer_print(ClientBuffer *);	
void ClientBuffer_add(ClientBuffer *, int);
int ClientBuffer_remove(ClientBuffer* CBptr);




#endif
