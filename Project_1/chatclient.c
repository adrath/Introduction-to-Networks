/******************************************************************************
* Author: Alexander Drath
* Date: 7/24/19
* Last Updated: 7/24/19
* Course: CS372 - Intro to Networks
* Assignment: Project 1
* Filename: chatserve.py
* Description: Design and implement a simple chat system that works for one pair 
*   of users, i.e., create two programs: a chat server and a chat client.  The
*   final version of your programs must accomplish the following tasks:
*       1. chatserve starts on host A. 
*       2. chatserve on host A waits on a port (specified by command-line) for 
*           a client request. 
*       3. chatclient starts on host B, specifying host A's hostname and port 
*           number on the command line. 
*       4. chatclient on host B gets the user’s "handle" by initial query (a 
*           one-word name, up to 10 characters).  chatclient will display this 
*           handle as a prompt on host B, and will prepend it to all messages 
*           sent to host A.  e.g.,  "SteveO> Hi!!" 
*       5. chatclient on host B sends an initial message to chatserve on host 
*           A : PORTNUM.  This causes a connection to be established between 
*           Host A and Host B.  Host A and host B are now peers, and may alternate 
*           sending and receiving messages.  Responses from host A should have 
*           host A's "handle" prepended. 
*       6. Host A responds to Host B, or closes the connection with the command "\quit" 
*       7. Host B responds to Host A, or closes the connection with the command "\quit" 
*       8. If the connection is not closed, repeat from 6. 
*       9. If the connection is closed, chatserve repeats from 2 
*           (until a SIGINT is received)
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define MAX_SIZE 500

/******************************************************************************
* Function: struct sockaddr_in setUpAddress(char* pn, char* user)
* Description: set up the server address structure. A lot of the socket set up
*   was taken from CS344 examples given by Benjamin Brewster. Specifically it came
*   from client.c file provided in Block 4 of CS344 course. Also taken from my
*   CS344 Project 4 - OTP. Please ask for my code if you would like to reference it.
* Input: argv[1] (portNumber), argv[2] (hostname)
* Output: struct sockaddr_in serverAddress
******************************************************************************/
struct sockaddr_in setUpAddress(char* user, char* pn){
    int portNumber;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;

    //Initialize the address structure by clearing it out
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    
    //Taken from CS344 client.c file provided by Benjamin Brewster in Block 4 of course
    portNumber = atoi(pn); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(user); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0);
    }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    //return the address to main
    return serverAddress;
}

/******************************************************************************
* Function: void getUsername(char* username)
* Description: get user handle from the user
* Input: char* username
* Output: char* username
******************************************************************************/
void getUsername(char* username){
    while (strlen(username) > 10 || strlen(username) == 0 || strstr(username, " ") != NULL){
	    memset(username, '\0', sizeof(username));
        printf("Please enter a 10 character user handle that will display on your messages.\n");
        fgets(username, sizeof(username) - 1, stdin);
	    username[strcspn(username, "\n")] = '\0';
    }
}

/******************************************************************************
* Function: int createSocket(struct sockaddr_in serverAddress)
* Description: set up the socket and double check that the socket created actually
*   connected to the server. Establish the connection as well.
* Input: struct sockaddr_in serverAddress
* Output: int socketFD
******************************************************************************/
int createSocket(struct sockaddr_in serverAddress){
    //create the socket
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0){
        perror("Client: Error opening socket\n");
        exit(1);
    }

    //connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        perror("Client: Error connecting to server\n");
        exit(1);
    }
    return socketFD;
}

/******************************************************************************
* Function: void exchangeUsername(int socketFD, char* username, char* serverName)
* Description: Exchange usernames between the server and the client so that they
*   can prepend the handle names before posting the messages
* Input: int socketFD, char* username, char* serverName
* Output: N/A
******************************************************************************/
void exchangeUsername(int socketFD, char* username, char* serverName){
    int sendUsername = send(socketFD, username, strlen(username), 0);
    if (sendUsername < 0){
        perror("Client: Error sending username to server\n");
        exit(1);
    }

    int getServerName = recv(socketFD, serverName, sizeof(serverName), 0);
    if (getServerName < 0){
        perror("Client: Error receiving username to server\n");
        exit(1);
    }
}


/******************************************************************************
* Function: void sendAndRecv(int socketFD, char* username, char* serverName)
* Description: send and receive messages from the server in a chat format. The
*   client must send the first message in order for the server to reply. When
*   wanting to quit, type in \quit and send that message before breaking from
*   breaking out of the loop in order to close the socket back in the main 
*   function.
* Input: int socketFD, char* username, char* serverName
* Output: N/A
******************************************************************************/
void sendAndRecv(int socketFD, char* username, char* serverName){
    int check;
    char outMessage[MAX_SIZE];
    char inMessage[MAX_SIZE];

    //Make infinite loop unless the connection is ended
    while(1){
        //initialize inMessage, outMessage, and check
        memset(outMessage, '\0', sizeof(outMessage));
        memset(inMessage, '\0', sizeof(inMessage));
        check = 0;
	
	//get input message from user
	while (strlen(outMessage) > 500 || strlen(outMessage) == 0){
        	printf("%s> ", username);
        	fgets(outMessage, sizeof(outMessage) - 1, stdin);
        	outMessage[strcspn(outMessage, "\n")] = '\0';
	}

        //send message to server
        check = send(socketFD, outMessage, sizeof(outMessage), 0);
        if (check < 0){
            perror("Client: Error sending message to server\n");
            exit(1);
        }
        if (check < strlen(outMessage)){
            printf("CLIENT: WARNING: Not all data written to socket!\n");
        }
	
	if (strstr(outMessage, "\\quit") != NULL){
		printf("Exiting chat with server\n");
		break;
	}
	
	//receive message from server
        check = 0;
        check = recv(socketFD, inMessage, sizeof(inMessage) - 1, 0);
        if (check < 0){
            perror("Client: Error receiving message from server\n");
            exit(1);
        }
        else if (check == 0){
            printf("The connection has been closed by the server\n");
            break;
        }
        else {
            printf("%s> %s\n", serverName, inMessage);
        }
    }    
}


/******************************************************************************
* Function: int main(int argc, char* argv[])
* Description: Refer to file description above.
******************************************************************************/
int main(int argc, char* argv[]){
    //Declare variables
    int socketFD;
    struct sockaddr_in serverAddress;
    char username[10];
    char serverName[10];

    //Check the correct number of arguments were given, if not error and exit
    if (argc < 3){
        fprintf(stderr, "Please enter in format: ./chatclient hostname [port]\n"); exit(1);
    }

    //Get username handle
    //memset(username, '\0', sizeof(username));
    //getUsername(username);
    
    while (strlen(username) > 10 || strlen(username) == 0 || strstr(username, " ") != NULL){
	    memset(username, '\0', sizeof(username));
        printf("Please enter a 10 character user handle that will display on your messages.\n");
        fgets(username, sizeof(username) - 1, stdin);
    }
    username[strcspn(username, "\n")] = '\0';

    //Set up the server address structure
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress = setUpAddress(argv[1], argv[2]);

    //create the socket and establish a connection to server
    socketFD = createSocket(serverAddress);

    //exchange usernames
    exchangeUsername(socketFD, username, serverName);

    //send and receive messages in a chat format
    sendAndRecv(socketFD, username, serverName);

    //Close the socket
    close(socketFD);

    return 0;
}


/******************************************************************************
* Sources:
* https://beej.us/guide/bgnet/
* CS344 Project 4 - OTP (completed project from Spring 2019)
* CS344 Lecture Notes for Module 4 (server.c and client.c)
* https://www.geeksforgeeks.org/socket-programming-cc/
* https://stackoverflow.com/questions/17131863/passing-string-to-a-function-in-c-with-or-without-pointers
******************************************************************************/
