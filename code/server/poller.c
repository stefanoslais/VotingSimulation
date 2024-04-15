#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h> 

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "thread_workers.h"		/*for  Worker Threads starting routine function and Parametres*/
#include "client_buffer.h"		/*Thread-Safe Structure that holds clients sockets*/
#include "poll_statistics.h"		/*for PartiesList structure*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

static int ServerPassiveSocket = -1;		/*Server listening passive socket*/
bool ReceiveNewClients = true;			/*global*/

void print_error(char const* mess,int res) {
	char error_buf[256] = {'\0'};
	strerror_r(res,error_buf,256);
	fprintf(stderr,"%s : %s\n",mess, error_buf);
}

void sigint_handler(int signal) {
	if(ServerPassiveSocket != -1 )	close(ServerPassiveSocket);
	ReceiveNewClients = false;
	puts("SIGINT was called");
}

int main(int argc, char* argv[]){

/*agv[0] is the name of the program*/
/*argv[1] is the port number the server will be listening to*/
int port = atoi(argv[1]);
/*argv[2] is the number of worker threads*/
int numWorkerThread = atoi(argv[2]);	
if(numWorkerThread <= 0 )  { perror("number of threads"); exit(EXIT_FAILURE); }
/*argv[3] is the number of clients waiting to be server*/
int bufferSize = atoi(argv[3]);
if(bufferSize <= 0 ) { perror("number of clients waiting to be server"); exit(EXIT_FAILURE); }
/*argv[4] is the name of the file that contains Voters and their Votes*/
char* PollLogptr = argv[4];
/*argv[5] is the name of the file that contains the Results of the Roll*/
char* PollStatsptr = argv[5];


/*Setting up signal handling for Master Thread*/
signal(SIGINT,sigint_handler);

ClientBuffer myClientBuffer;
ClientBuffer_initialize(&myClientBuffer,bufferSize);
PartiesList myPartiesList;
PartiesList_initialize(&myPartiesList);

/*creating an array that holds all the the Worker Threads*/
pthread_t* WorkerThreads_arr = (pthread_t *) malloc(numWorkerThread * sizeof(pthread_t));
/*creating an array that holds all the parametres of Worker Threads*/
struct WorkerThreadsParams* WorkerThreadsParams_arr = malloc( numWorkerThread * sizeof(struct WorkerThreadsParams) );

/*for ecery thread...*/
for(int worker = 0; worker < numWorkerThread ; worker++ ) {
	int result;
/*initialize its parametres...*/
	WorkerThreadsParams_arr[worker].rank = worker;
	WorkerThreadsParams_arr[worker].CBptr = &myClientBuffer;
	WorkerThreadsParams_arr[worker].PollLog = PollLogptr;
	WorkerThreadsParams_arr[worker].PLptr = &myPartiesList;
/*and call routing function*/
	if((result = pthread_create(&WorkerThreads_arr[worker], NULL, thread_f, (void*)&WorkerThreadsParams_arr[worker])) != 0 ) {
		print_error("Thread Creation",result);
		exit(EXIT_FAILURE);
	}
}

/*Preparing communication with clients: */

/*Step 1: Creating the server socket*/
if( (ServerPassiveSocket = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
	perror("passive socket creation");
	exit(EXIT_FAILURE);
}


/*Step 2: Binding the socket to associate it with a spesific network addrress*/
struct sockaddr_in ServerAddress;
ServerAddress.sin_family = AF_INET;
ServerAddress.sin_addr.s_addr = INADDR_ANY;
ServerAddress.sin_port = htons(port);

if( bind(ServerPassiveSocket,(struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0 ) {
	perror("passive socket binding");
	exit(EXIT_FAILURE);
}

/*Step 3: Listening for the max number of connections that can be queued for acceptance*/ 
if( listen(ServerPassiveSocket, bufferSize) < 0 ) {
	perror("passive socket listening");
	exit(EXIT_FAILURE);
}
printf("Master Thread is listening to Passive Socket %d\n",ServerPassiveSocket);

/*Master Threads keeps on accepting clients as long as there are clients to be accepted.
When SIGINT is sent, Master Thread stops listening.
We do however want to service those who have arrived in the listening queue,
so we keep on accepting those. After they've been accepted, accept() returns -1*/

int ClientSocket;
while( (ClientSocket = accept(ServerPassiveSocket, NULL, NULL)) !=  -1 ) {
/*as long as the are accepted clients, add them to the Clients Buffer to be taken care by the worker threads*/
	ClientBuffer_add(&myClientBuffer,ClientSocket);
}

/*In case accept returned with an error due to unforseen events ( and not a SIGINT signal ) , global boolean ReceiveNewClients
would still be true */
if( ReceiveNewClients == true ) {
	printf("ReceiveNewClients: %d",(int) ReceiveNewClients);
	perror("Master Thread accepting client");
	close(ServerPassiveSocket);
	exit(EXIT_FAILURE);
}
/*Now that we have ensured that SIGINT signal was sent, we close ClientBuffer*/
ClientBuffer_close(&myClientBuffer,numWorkerThread);

/*Join Thread Workers*/
for(int worker = 0; worker < numWorkerThread; worker++) { 
	int result = 0;
	int status;
	if( (result = pthread_join(WorkerThreads_arr[worker],(void*) &status) ) != 0 ) {
		print_error("Thread Join",status);
		exit(EXIT_FAILURE);
	}
}

/*We destroy our structures and write out our voting results*/
ClientBuffer_destroy(&myClientBuffer);
PartiesList_appendtoFile(&myPartiesList, PollStatsptr);
PartiesList_destroy(&myPartiesList);

free(WorkerThreads_arr); WorkerThreads_arr = NULL;
free(WorkerThreadsParams_arr); WorkerThreadsParams_arr = NULL;

return 0;
}
