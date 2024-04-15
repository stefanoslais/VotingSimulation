#ifndef __STRING_MANIPULATION_H__
#define __STRING_MANIPULATION_H__

#include <stdbool.h>
/*Some useful functions regarding string manipulation*/
void concatenate_NameSurname(char** Buffer, char* Name, char* SurName);
void parseLine(char* Line,char** VoterName, char** VoterSurname, char** VoterParty, bool WithVoterParty);


#endif

