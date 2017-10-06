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
	

// This method takes in the and computations, a pointer to the computetd results, a udp port number, and 
// the number of and computations. It sends the data to the backend AND server, receives the reulsts, and
// stores them in andResults
void computeAnd(char * andBuffer, char *andResults, int udpSocket, int andCount) {
    int andPort, andSocket;
    struct sockaddr_in andServAddr;
    socklen_t addr_size;

    addr_size = sizeof andServAddr;
    andPort = 22398; //port number 22000+398
    andServAddr.sin_family = AF_INET;
    andServAddr.sin_port = htons(andPort);
    andServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(andServAddr.sin_zero, '\0', sizeof andServAddr.sin_zero);
    sendto(udpSocket, andBuffer, strlen(andBuffer), 0, (struct sockaddr * ) & andServAddr, addr_size);
    printf("The edge server has successfully sent %d lines to Backend-Server AND.\n", andCount);
    recvfrom(udpSocket, andResults, 102400, 0, (struct sockaddr * ) & andServAddr, &addr_size);
}

// This method takes in the or computations, a pointer to the computetd results, a udp port number, and 
// the number of or computations. It sends the data to the backend OR server, receives the reulsts, and
// stores them in orResults
void computeOr(char * orBuffer, char *orResults, int udpSocket, int orCount) {
    int orPort, orSocket;
    struct sockaddr_in orServAddr;
    socklen_t addr_size;

    addr_size = sizeof orServAddr;
    orPort = 21398; // port number 21000+398
    orServAddr.sin_family = AF_INET;
    orServAddr.sin_port = htons(orPort);
    orServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(orServAddr.sin_zero, '\0', sizeof orServAddr.sin_zero);
    sendto(udpSocket, orBuffer, strlen(orBuffer), 0, (struct sockaddr * ) & orServAddr, addr_size);
    printf("The edge server has successfully sent %d lines to Backend-Server OR.\n", orCount);
    recvfrom(udpSocket, orResults, 102400, 0, (struct sockaddr * ) & orServAddr, &addr_size);

}

// This method prints out the computation results sent back from backend servers and prints them out
// in a format(excludes the line number and dash)
void printWithoutOrder(char * andResult, char * orResult) {
	char * curLine, * buf, * tokenStr1, * tokenStr2;
	char copy[1024];
	
	curLine = NULL;
	buf = NULL;
	tokenStr1 = NULL;
	tokenStr2 = NULL;

	// prints out results from OR server
	strcpy(copy, orResult);
	curLine = strtok_r(copy, "\n", &tokenStr1);
	while(curLine != NULL) {
		strtok(curLine, "-");
		buf = strtok(NULL, "");	
		printf("%s\n", buf);
		curLine = strtok_r(NULL, "\n", &tokenStr1); 
	}

	// prints out results from AND server
	strcpy(copy, andResult);
	curLine = strtok_r(copy, "\n", &tokenStr2);
	while(curLine != NULL) {
		strtok(curLine, "-");
		buf = strtok(NULL, "");	
		printf("%s\n", buf);
		curLine = strtok_r(NULL, "\n", &tokenStr2); 
	}
}

// The main method
int main() {
	int parentSocket, childSocket, udpSocket;
    int andCount, orCount, tcpPort, udpPort;
    char buffer[102400], andBuffer[102400], orBuffer[102400], andResult[102400], orResult[102400], finalResults[102400];
    char type[4], countStr[1024];
    char * line, * message, * tokenStr;
    struct sockaddr_in edgeServerAddr, edgeClientAddr;
    struct sockaddr_storage clientAddr;
    socklen_t addr_size;

    tcpPort = 23398;
    udpPort = 24398;

    // Binds a TCP socket
    parentSocket = socket(PF_INET, SOCK_STREAM, 0);
    edgeServerAddr.sin_family = AF_INET;
    edgeServerAddr.sin_port = htons(tcpPort);
    edgeServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(edgeServerAddr.sin_zero, '\0', sizeof edgeServerAddr.sin_zero);
    bind(parentSocket, (struct sockaddr * ) & edgeServerAddr, sizeof(edgeServerAddr));

    // Binds a UDP socket
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    edgeClientAddr.sin_family = AF_INET;
    edgeClientAddr.sin_port = htons(udpPort);
    edgeClientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(edgeClientAddr.sin_zero, '\0', sizeof edgeClientAddr.sin_zero);
    bind(udpSocket, (struct sockaddr * ) & edgeClientAddr, sizeof(edgeClientAddr));

    // Start listening for incoming connections
    if (listen(parentSocket, 5) == 0) {
        printf("The edge server is up and running.\n");
    }
    while (1) {
		memset(buffer, 0, sizeof(char) * 102400);
		memset(andBuffer, 0, sizeof(char)* 102400);
		memset(orBuffer, 0, sizeof(char)* 102400);
		memset(andResult, 0, sizeof(char)* 102400);
		memset(orResult, 0, sizeof(char)* 102400);
		memset(type, 0, sizeof(char)* 4);
		memset(countStr, 0, sizeof(char)* 1024);
		memset(finalResults, 0, sizeof(char)* 102400);

		message = NULL;
		line = NULL;
		tokenStr = NULL;
    	andCount = 0;
    	orCount = 0;

        addr_size = sizeof clientAddr;
        // Creates the child socket for the client TCP connection
        childSocket = accept(parentSocket, (struct sockaddr * ) &clientAddr, &addr_size);
        if (!fork()) {
            recv(childSocket, buffer, 102400, 0);
            line = strtok_r(buffer, "\n", & tokenStr);
            while (line != NULL) {
            	// Break received results into lines and divide them into andBuffer and orBuffer accordingly
            	// Also adds a line number to each line, 1-xxxxx, 2-xxxxx, where xxxxx is the actual computation part.
                strcpy(type, strtok(line, ","));
                message = strtok(NULL, "");
                memset(countStr, 0, 1024);
                if (strcmp(type, "and") == 0) {
                	andCount ++;
                	sprintf(countStr, "%d", andCount + orCount);
                	strcat(andBuffer, countStr);
                	strcat(andBuffer, "-");
                    strcat(andBuffer, message);
                    strcat(andBuffer, "\n");
                } else {
                    orCount ++;
                    sprintf(countStr, "%d", andCount + orCount);
                    strcat(orBuffer, countStr);
                    strcat(orBuffer, "-");
                    strcat(orBuffer, message);
                    strcat(orBuffer, "\n");
                }
                line = strtok_r(NULL, "\n", & tokenStr);
            }
            printf("The edge server has received %d lines from the client using TCP over port %d\n", andCount + orCount, tcpPort);
            
            // Calls individual methods that handle AND connection and OR connection
            computeOr(orBuffer, orResult, udpSocket, andCount);            
            computeAnd(andBuffer, andResult, udpSocket, orCount);
            printf("The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP over port %d.\n", udpPort);
            printf("The computation results are:\n");

            // Prints out received results.
            printWithoutOrder(andResult, orResult);

            // Combine the results and send back to the client
            strcat(finalResults, andResult);
            strcat(finalResults, orResult);
            printf("The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server AND.\n");
            if(send(childSocket, finalResults, 102400, 0) >= 0) {
            	printf("The edge server has successfully finished sending all computation results to the client.\n");
        	}
        }else {
        	close(childSocket);
        }
    }
    return 0;
}