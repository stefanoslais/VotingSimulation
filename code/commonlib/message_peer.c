#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "message_peer.h"


void initialize_MessageBuffer(char* MessageBuffer, int Length) {
	memset(MessageBuffer,'\0', Length);
}

/*a wrapper function to execute  proper send() guaranteeing that all bytes have been sent*/
void send_toPeer(int socket, char* MessageBuffer,int MAX_L) {
	int BytesSent = 0;
	int RemainingBytes = MAX_L;
	while( RemainingBytes > 0 ) { 		/*as long as there are remaining bytes to be sent*/
		BytesSent = send(socket, MessageBuffer + BytesSent, MAX_L - BytesSent, 0);
		if(BytesSent == -1) {
			fprintf(stderr,"sending bytes: %s\n",strerror(errno));
		 	exit(EXIT_FAILURE);
		}
		if(BytesSent == 0) break;
		RemainingBytes = RemainingBytes - BytesSent;
	}
	return ;
}

void free_Voter(char** VoterName, char** VoterSurname, char** VoterParty) {
	free(*VoterName);
	free(*VoterSurname);
	free(*VoterParty);
	*VoterName = NULL;
	*VoterSurname = NULL;
	*VoterParty = NULL;
}

void	free_MessageBuffer(char** MessageBuffer) {
	free(*MessageBuffer);
	*MessageBuffer = NULL;
}
