#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>	     /* internet sockets */
#include <unistd.h>          /* read, write, close */
#include <netdb.h>	     /* gethostbyaddr */
#include <string.h>	     /* strlen */
#include <errno.h>
#include <stdlib.h>	     /* exit */
#include <stdio.h>
#include <pthread.h>

#include "client_side.h" 		
#include "../commonlib/message_peer.h"		/*for send_toPeer() */
#include "../commonlib/string_manipulation.h"	/*for parseLine() */

#define MESS_SZ 100 
void ClientThreadsParams_set(ClientThreadsParams* CTP ,size_t rank , char* ServerName,int ServerPort, char* Line) {
	CTP->myrank = rank;
	CTP->ServerName = ServerName;
	CTP->portNum = ServerPort;
	CTP->Line = malloc(strlen(Line) + 1);
	strcpy(CTP->Line,Line);
	/*get rid of newline character*/
	strtok(CTP->Line,"\n");
	return ;
}

void* client_thread_func(void* ClientParametres) {

	ClientThreadsParams* CTP = (ClientThreadsParams *) ClientParametres;
	size_t myrank = CTP->myrank;
	char* ServerName = CTP->ServerName;
	int ServerPort = CTP->portNum;
	char* Line = CTP->Line;
	
//printf("Thread %lu: Id: %lu ServerName: %s ServerPort: %d Line: %s\n",myrank,pthread_self(),ServerName,ServerPort,Line);

/*Establishing connection...*/
/*Step 1: Create the socket*/
int ClientSocket;
if( (ClientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	perror("client side socket creation");
	exit(EXIT_FAILURE);
}

/*Step 2:Trying to find the server...*/
struct hostent* Server_temp;
if( (Server_temp = gethostbyname(ServerName)) == NULL) {
	herror("failed to resolve server name");
	exit(EXIT_FAILURE);
}

/*Step 3:Server found, let's save its info*/
struct sockaddr_in myServer;
myServer.sin_family = AF_INET;
memcpy(&myServer.sin_addr, Server_temp->h_addr, Server_temp->h_length);
myServer.sin_port = htons(ServerPort);
/*Server info is now fullfilled, we are ready to connect*/


/*Step 4: Connect*/
if( connect(ClientSocket,(struct sockaddr*) &myServer,sizeof(myServer)) < 0 ) {
	perror("client connect");
	exit(EXIT_FAILURE);
}

/*Client-Server exchange messaging starts here */
	char* MessageFromServer = malloc(MESS_SZ * sizeof(char));
	char* MessageFromClient = malloc(MESS_SZ * sizeof(char));
	initialize_MessageBuffer(MessageFromServer, MESS_SZ);	
	initialize_MessageBuffer(MessageFromClient, MESS_SZ);	


/*Client receives "SEND NAME PLEASE"*/
	recv(ClientSocket, MessageFromServer, MESS_SZ, MSG_WAITALL);
	printf("Thread %lu: Message from Server: %s\n", myrank, MessageFromServer);

/*Get name surname and party from Line. The following 3 varriables are dynamically allocated inside parseLine()*/
	char* VoterName;
	char* VoterSurname;
	char* VoterParty;
	parseLine(Line, &VoterName, &VoterSurname, &VoterParty, true);

/*Populate MessageFromClient with name and surname. MessageFromClient is definately null terminated*/
	strcpy(MessageFromClient, VoterName);
	strcat(MessageFromClient, " ");
	strcat(MessageFromClient, VoterSurname);

/*Send name and surname*/
	send_toPeer(ClientSocket, MessageFromClient, MESS_SZ);

/*Receive Message From Server*/
	initialize_MessageBuffer(MessageFromServer, MESS_SZ);
	recv(ClientSocket, MessageFromServer, MESS_SZ, MSG_WAITALL);
	printf("Thread %lu: Message from Server: %s\n", myrank, MessageFromServer);

/*If Server sent "SEND VOTE PLEASE"*/
	if( strcmp(MessageFromServer,"SEND VOTE PLEASE") == 0 ) {
/*Populate MessageFromClient with VoterParty. MessageFromClient is definately null terminated*/
		initialize_MessageBuffer(MessageFromClient, MESS_SZ);
		strcpy(MessageFromClient, VoterParty);	
/*Send Voter Party*/
		send_toPeer(ClientSocket,MessageFromClient, MESS_SZ);

/*Wait confirmation message*/
		initialize_MessageBuffer(MessageFromServer, MESS_SZ);
		recv(ClientSocket, MessageFromServer, MESS_SZ, MSG_WAITALL);
		printf("Thread %lu: Message from Server: %s\n", myrank, MessageFromServer);
	}
	else if( strcmp(MessageFromServer,"ALREADY VOTED") == 0 )
	{
		/*Do nothing*/
	}

	free_Voter(&VoterName, &VoterSurname, &VoterParty);
	free_MessageBuffer(&MessageFromServer);
	free_MessageBuffer(&MessageFromClient);

	close(ClientSocket);
	pthread_exit(NULL);
	return NULL;
}


















		
