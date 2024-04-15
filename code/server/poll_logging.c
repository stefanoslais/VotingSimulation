#include "poll_logging.h"
#include "../commonlib/string_manipulation.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>		/*for open() */
#include <unistd.h>		/*for close() */

#define MESS_SZ 100 
void MultiThreadFileRW_initialize(MultiThreadFileRW* File, char* fileptr) 
{ 

	File->FileName = fileptr;
	pthread_mutex_init(&(File->FileAccess_lock), NULL);
	pthread_cond_init(&(File->ThreadCanWrite_cond), NULL);
	pthread_cond_init(&(File->ThreadCanRead_cond), NULL);
	File->ActiveWriters = 0;
	File->WaitingWriters = 0;
	File->ActiveReaders = 0;
	File->WaitingReaders = 0;
}

/*There are reader threads and writer threads. At first , reader threads search poll Log file for the name*/
bool FindVoter(MultiThreadFileRW* File, char* VoterName, char* VoterSurname) {

/*As long as there are no writers holding the structure OR waiting to get a hold of it, search the file
Priority is clearly given to writers for purpose of keeping poll log realistic and as updated as possible.
Active writers do NOT intervene with active readers*/
	pthread_mutex_lock(&(File->FileAccess_lock));
	while( File->ActiveWriters + File->WaitingWriters > 0 ) {
		(File->WaitingReaders)++;
		pthread_cond_wait(&(File->ThreadCanRead_cond),&(File->FileAccess_lock));
		(File->WaitingReaders)--;
	}
	(File->ActiveReaders)++;
	pthread_mutex_unlock(&(File->FileAccess_lock));


/*SEARCH STARTS HERE...*/

/*Each Thread opens up its own File Stream so that the File Cursor doesn't get messed up between thread operations*/ 
/*However, fopen() for reading purposes doesn't create the file unless it already exist.
For this reason we open the file with open() and correlate the file descriptor with a file stream through fdopen() function */
	int LocalFilefd;
	if( (LocalFilefd = open(File->FileName, O_CREAT | O_RDONLY,				      \
						S_IRUSR | S_IRGRP | S_IROTH | /*for waiting readers */ \
						S_IWUSR | S_IWGRP | S_IWOTH )/*for active writer   */ \
	    
	    ) == -1)
	{
		perror("opening file descriptor to read from Poll Log");
		exit(EXIT_FAILURE);
	}
	FILE* LocalStream;
	if( (LocalStream = fdopen(LocalFilefd,"r")) == NULL ) {
		perror("opening file stream for reading Poll Log"); 
		exit(EXIT_FAILURE);
	}

	rewind(LocalStream);
	char Line[MESS_SZ] = { '\0' };
/*Because every line in pollLog.txt ends with a newline, fgets will always return successully except when it reaches EOF*/
	char* LineName;
	char* LineSurName;
	bool  VoterFound = false;
/*As long as there is a line to read from the file and the Voter has not been found*/
	while( fgets(Line,MESS_SZ,LocalStream) != NULL && VoterFound == false) {
/*Extract Name and Surname from the Line*/
		parseLine(Line,&LineName,&LineSurName,NULL, false);		
/*If it matches to the Voter's Name and SurName, set the flag to true*/
		if(strcmp(LineName,VoterName) == 0 && strcmp(LineSurName,VoterSurname) == 0 ) {
			VoterFound = true;
		}
		free(LineName);
		free(LineSurName);
	}
	fclose(LocalStream);
	close(LocalFilefd);
/*SEARCH ENDS HERE...*/
	pthread_mutex_lock(&(File->FileAccess_lock));
	(File->ActiveReaders)--;
/*As long as there are no active readers and there are writers waiting, broadcast writers*/
	if( File->ActiveReaders == 0 && File->WaitingWriters > 0 )
		pthread_cond_broadcast(&(File->ThreadCanWrite_cond));
	pthread_mutex_unlock(&(File->FileAccess_lock));
	
	return VoterFound;
}

void AppendVote(MultiThreadFileRW* File,char* VoterName, char* VoterSurname, char* VoterParty) {
	
/*Only one writer can write to poll log file as long as there are no active readers*/
	pthread_mutex_lock(&(File->FileAccess_lock));
	while( File->ActiveWriters + File->ActiveReaders > 0 ) {
		(File->WaitingWriters)++;
		pthread_cond_wait(&(File->ThreadCanWrite_cond),&(File->FileAccess_lock));
		(File->WaitingWriters)--;
	}

	(File->ActiveWriters)++;
	pthread_mutex_unlock(&(File->FileAccess_lock));

/*WRITTING STARTS HERE*/
	int LocalFilefd;
	if( (LocalFilefd = open(File->FileName, O_CREAT | O_WRONLY | O_APPEND | O_SYNC,		      \
						S_IRUSR | S_IRGRP | S_IROTH | /*for waiting readers */ \
						S_IWUSR | S_IWGRP | S_IWOTH )/*for active writer   */ \
	    ) == -1) {
		perror("opening file descriptor to write to Poll Log");
		exit(EXIT_FAILURE);
	}
	FILE* LocalStream;
	if( (LocalStream = fdopen(LocalFilefd,"a")) == NULL ) {
		perror("opening file stream for writting to  Poll Log"); 
		exit(EXIT_FAILURE);
	}
	fprintf(LocalStream, "%s %s %s\n", VoterName, VoterSurname, VoterParty);
	fclose(LocalStream);
	close(LocalFilefd);
/*WRITTING ENDS HERE*/

	pthread_mutex_lock(&(File->FileAccess_lock));
	(File->ActiveWriters)--;
/*If there are Writers waiting, give them priority. If there aren't , allow readers */
	if( File->WaitingWriters > 0 )
		pthread_cond_broadcast(&(File->ThreadCanWrite_cond));
	else if ( File->WaitingReaders > 0 ) 
		pthread_cond_broadcast(&(File->ThreadCanRead_cond));
	
	pthread_mutex_unlock(&(File->FileAccess_lock));
	
}





