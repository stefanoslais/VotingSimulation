#ifndef __THREAD_WORKERS_H__
#define __THREAD_WORKERS_H__

struct ClientBuffer_s;
struct PartiesList_s;

struct WorkerThreadsParams{
	int rank;				/*an id for each thread*/
	struct ClientBuffer_s* CBptr;		/*pointer to the client buffer list*/
	char* PollLog;				/*pointer to poll log file name*/
	struct PartiesList_s* PLptr;		/*pointer to structure that holds election results*/
};

void* thread_f(void * );






#endif
