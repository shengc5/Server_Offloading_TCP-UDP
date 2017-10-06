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
    int orSocket, port;
    int numOne, numTwo, orResult, count;
    char buffer[102400], results[102400], opOne[1024], opTwo[1024], newLine[1024], orResultStr[1024], buf[1024];
    char * line;
    char * num;
    char * end_str;
    char * copyLine;

    struct sockaddr_in orServer;
    struct sockaddr_storage clientAddr;
    socklen_t addr_size;

    // Creates and binds a socket for OR server
    orSocket = socket(PF_INET, SOCK_DGRAM, 0);
    port = 21398;
    orServer.sin_family = AF_INET;
    orServer.sin_port = htons(port);
    orServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(orServer.sin_zero, '\0', sizeof orServer.sin_zero);
    bind(orSocket, (struct sockaddr *) &orServer, sizeof(orServer));
    addr_size = sizeof clientAddr;
    printf("The Server OR is up and running using UDP on port %d.\n", port);

    while (1) {
		memset(buffer, 0, 102400);
		memset(results, 0, 102400);

		count = 0;
        recvfrom(orSocket, buffer, 1024, 0, (struct sockaddr *) &clientAddr, &addr_size);
        printf("The Server OR start receiving lines from the edge server for OR computation.The computation results are:\n");
        line = strtok_r(buffer, "\n", &end_str);
        
        // process the received lines, prints out results on screen, and sends back to the edge
        while (line != NULL) {
            count ++;
            newLine[0] = '\0';
            memset(buf, 0, 1024);
            num = strtok(line, "-");
            strcpy(opOne, strtok(NULL, ","));
            strcpy(opTwo, strtok(NULL, ""));
            numOne = atoi(opOne);
            numTwo = atoi(opTwo);
            orResult = decimalToBinary(binaryToDecimal(numOne) | binaryToDecimal(numTwo));
            sprintf(orResultStr, "%d", orResult);
            strcat(newLine, num);
            strcat(newLine, "-");            
            strcat(newLine, opOne);
            strcat(newLine, " or ");
            strcat(newLine, opTwo);
            strcat(newLine, " = ");
            strcat(newLine, orResultStr);
            strcat(results, newLine);
            strcat(results, "\n");

            strcat(buf, opOne);
            strcat(buf, " or ");
            strcat(buf, opTwo);
            strcat(buf, " = ");
            strcat(buf, orResultStr);
            printf("%s\n", buf);
            line = strtok_r(NULL, "\n", &end_str);
        }
        printf("The Server OR has successfully received %d lines from the edge server and finished all OR computations.\n", count);
        sendto(orSocket, results, strlen(results), 0, (struct sockaddr *) &clientAddr, addr_size);
        printf("The Server OR has successfully finished sending all computation results to the edge server\n");
    }
    return 0;
}
