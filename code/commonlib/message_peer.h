#ifndef __MESSAGE_PEER_H__
#define __MESSAGE_PEER_H__



/*Some helping functions regarding the exchange of messages between server and client*/
void initialize_MessageBuffer(char* MessageBuffer, int Length);

void send_toPeer(int socket, char* MessageBuffer,int MAX_L);

void free_Voter(char** VoterName, char** VoterSurname, char** VoterParty);

void free_MessageBuffer(char** MessageBuffer);








#endif
