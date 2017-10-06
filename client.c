#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>

// This method prints out all computations results received from the edge server according to the line 
// number appended in front of each line
void printSorted(char * buf, int count) {
	char lines[count][1024];
	char * curLine;
	char * tokenStr;
	char * copy;
	char * operation;
	char * result;

	operation = NULL;
	result = NULL;
	curLine = NULL;
	tokenStr = NULL;
	copy = NULL;

	// Store all lines in a String array, index is the line number of that specific line.
	curLine = strtok_r(buf, "\n", &tokenStr);
	char * num = strtok(curLine, "-");
	operation = strtok(NULL, "=");
	result = strtok(NULL, "");
	int order = atoi(num);
	strcpy(lines[order], result);
	for(int i = 1; i < count; i++) {
		curLine = strtok_r(NULL, "\n", &tokenStr);
		char * num = strtok(curLine, "-");
		operation = strtok(NULL, "=");
		result = strtok(NULL, "");
		int order = atoi(num);
		strcpy(lines[order], result);
	}

	// Prints them out
	for(int j = 1; j <= count; j++) {
		char * noSpace = lines[j] + 1;
		printf("%s\n", noSpace);
	}
}

int main( int argc, char *argv[] ){
	int clientSocket, edgePort, count;
	char line[10240], buffer[102400];
	struct sockaddr_in serverAddr;
	socklen_t addr_size;
	FILE *fp;

	memset(buffer, 0, 102400);
	memset(line, 0, 10240);

	// Creates a socket for TCP connection
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  	edgePort = 23398;
  	count = 0;
  	serverAddr.sin_family = AF_INET;
  	serverAddr.sin_port = htons(edgePort);
  	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  	addr_size = sizeof serverAddr;

  	// Once connected, start file reading the sends the computations over to the edge server
  	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
    printf("The client is up and running\n");
	fp = fopen(argv[1], "r");
	while (fgets(line, 10240, fp)) {
		count = count + 1;
		strcat(buffer, line);
	}
	send(clientSocket,buffer, 102400,0);
	printf("The client has successfully finished sending %d lines to the edge server.\n", count);
	memset(buffer, 0, strlen(buffer));

	// recevie the results and call printSorted to print computation results.
	recv(clientSocket, buffer, 102400, 0);
	printf("The client has successfully finished receiving all computation results from the edge server.");
	printf("The final computation results are:\n");
	printSorted(buffer, count);
}
