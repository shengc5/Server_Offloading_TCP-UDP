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

// converts a binary number (e.g. 10110) to its decimal value
int binaryToDecimal(int num) {
    int remainder, result = 0, base = 1;
    while (num > 0) {
        remainder = num % 10;
        result = result + remainder * base;
        num = num / 10;
        base = base * 2;
    }
    return result;
}

// converts a decimal number to its binary form
int decimalToBinary(int num) {
    int remainder, result = 0, base = 1; 
	while (num != 0) {
        remainder = num % 2;
        num = num / 2;
        result = result + (remainder * base);
        base = base * 10;
    }
    return result;
}

int main() {
	int andSocket, port;
	int numOne, numTwo, andResult, count;
	char buffer[102400], results[102400], opOne[1024], opTwo[1024], newLine[1024], andResultStr[1024], buf[1024];
	char * line;
	char * end_str;
	char * copyLine;
	char * num;
	struct sockaddr_in andServer;
	struct sockaddr_storage clientAddr;
	socklen_t addr_size;

	// Creates and binds a socket for AND server
	andSocket = socket(PF_INET, SOCK_DGRAM, 0);
	port = 22398;
   	andServer.sin_family = AF_INET;
  	andServer.sin_port = htons(port);
  	andServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(andServer.sin_zero, '\0', sizeof andServer.sin_zero);  
	bind(andSocket, (struct sockaddr *) &andServer, sizeof(andServer));
	addr_size = sizeof clientAddr;
	printf("The Server AND is up and running using UDP on port %d.\n", port);
	while (1) {
		memset(buffer, 0, 102400);
		memset(results, 0, 102400);
		count = 0;
		recvfrom(andSocket, buffer, 1024, 0, (struct sockaddr *) &clientAddr, &addr_size);
		printf("The Server AND start receiving lines from the edge server for AND computation.The computation results are:\n");
		line = strtok_r(buffer, "\n", &end_str);
   		// process the received lines, prints out results on screen, and sends back to the edge
   		while (line != NULL) {
            count ++;
            newLine[0] = '\0';
            memset(buf, 0, 1024);
            num = strtok(line, "-");
            strcpy(opOne, strtok(NULL, ","));
            strcpy(opTwo, strtok(NULL, ","));
            numOne = atoi(opOne);
            numTwo = atoi(opTwo);
            andResult = decimalToBinary(binaryToDecimal(numOne) & binaryToDecimal(numTwo));
            sprintf(andResultStr, "%d", andResult);
            strcat(newLine, num);
            strcat(newLine, "-");
            strcat(newLine, opOne);
            strcat(newLine, " and ");
            strcat(newLine, opTwo);
            strcat(newLine, " = ");
            strcat(newLine, andResultStr);
            strcat(results, newLine);
            strcat(results, "\n");

            strcat(buf, opOne);
            strcat(buf, " and ");
            strcat(buf, opTwo);
            strcat(buf, " = ");
            strcat(buf, andResultStr);
            printf("%s\n", buf);

            line = strtok_r(NULL, "\n", &end_str);
		}
		printf("The Server AND has successfully received %d lines from the edge server and finished all AND computations.\n", count);
		sendto(andSocket, results, strlen(results), 0, (struct sockaddr *) &clientAddr, addr_size);
		printf("The Server AND has successfully finished sending all computation results to the edge server\n");
	}
	return 0;
}