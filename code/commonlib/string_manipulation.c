#include "string_manipulation.h"
#include <string.h>
#include <stdlib.h>

void concatenate_NameSurname(char** Buffer, char* Name, char* SurName) {
	*Buffer = malloc(strlen(Name) + 1 /*for space*/ + strlen(SurName) + 1 /*for null terminator*/);
	strcpy(*Buffer,Name);
	strcat(*Buffer," ");
	strcat(*Buffer,SurName);
}

void parseLine(char* Line,char** VoterName, char** VoterSurname, char** VoterParty, bool WithVoterParty) {
	char* LineCopy = malloc(strlen(Line) + 1);
	strcpy(LineCopy,Line);
	char* saveptr = NULL; //for strtok purposes
	char* element = NULL;
/*Parse Voter Name*/
	element = strtok_r(LineCopy," ",&saveptr);
	*VoterName = malloc((strlen(element) + 1) * sizeof(char)) ;
	strcpy(*VoterName,element);
/*Parse Voter Surname*/
	element = strtok_r(NULL," ",&saveptr);
	*VoterSurname = malloc((strlen(element) + 1) * sizeof(char));
	strcpy(*VoterSurname,element);
/*Parse Voter Party if WithVoterParty is true.*/
	if(WithVoterParty == false)	return;

	element = strtok_r(NULL,"\0",&saveptr);
	*VoterParty = malloc( (strlen(element) + 1) * sizeof(char));
	strcpy(*VoterParty,element);

	free(LineCopy);
	return ;
}
