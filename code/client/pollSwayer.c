#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>

#include "client_side.h"

#define LINE_SIZE 100

int main(int argc , char* argv[]) {

/*argv[0] is the name of the executable*/
/*argv[1] is the name of the Server we want the client to conenct to*/
char* ServerName = malloc(strlen(argv[1]) + 1);
strcpy(ServerName,argv[1]);
/*argv[2] is the port number of the server*/
int ServerPort = atoi(argv[2]);
/*argv[3] is the name of the file we want to read from*/
char* FileName = argv[3];

/*Let's open the input file for reading.
Input file could be created with the help of ../BashScripts/create_input.sh */
FILE* inputFile = fopen(FileName,"r");
if( inputFile == NULL ) {
	perror("input file opening");
	exit(EXIT_FAILURE);
}

/*Find out how many lines there are in this file*/
char VoteBuffer[LINE_SIZE] = { '\0' } ;
size_t NumLines = 0;
while(fgets(VoteBuffer,LINE_SIZE,inputFile) != NULL )	NumLines++;

/*create an array of parametres. Each element corresponds to the parametres of a thread*/
ClientThreadsParams*  CTP_arr = malloc( NumLines * sizeof(ClientThreadsParams));;

/*Need to return the file cursor to the begging of the file (due to fgets)*/
fseek(inputFile,0,SEEK_SET);
char EachLine[LINE_SIZE] = { '\0' };

/*initialize thread parametres. Each thread receivs one line for inputFile*/
for(size_t ThisParam = 0; ThisParam < NumLines; ThisParam++) {
	fgets(EachLine,LINE_SIZE,inputFile);
	ClientThreadsParams_set(&CTP_arr[ThisParam], ThisParam, ServerName, ServerPort,EachLine);
}
/*Now the client thread parametres are all set. We are ready to create the threads*/
/*Create an array to hold all the threads, and called them*/
pthread_t* ClientThreads_arr = malloc(NumLines * sizeof(pthread_t));
for(size_t ThisThread = 0; ThisThread < NumLines; ThisThread++) {
	pthread_create(&(ClientThreads_arr[ThisThread]), NULL, client_thread_func, (void *)&CTP_arr[ThisThread]);
}

void* results;
for(size_t ThisThread = 0; ThisThread < NumLines; ThisThread++) { 
	if(pthread_join(ClientThreads_arr[ThisThread], &results) != 0) {
		perror("thread join problem");
		printf("%d\n", * (int*) results);
		exit(EXIT_FAILURE); }
}

/*Free ClientThreadsParam_arr*/
for(size_t ThisParam = 0; ThisParam < NumLines; ThisParam++) 	free(CTP_arr[ThisParam].Line);
free(CTP_arr);

/*Free ClientThreads_arr*/
free(ClientThreads_arr);
/*Rest of free() */
free(ServerName);
return 0;
}
