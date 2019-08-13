/*********************************************************************
* Author: Alexander Drath
* Course: CS372 - Intro to Networks
* Assignment: Project 2
* Filename: ftserver.c
* Date Created: 8/5/19
* Last Updated: 8/11/19
*
* Description:
*    Design and implement a simple file transfer system, i.e., create 
*    a file transfer server and a file transfer client. Write the 
*    ftserver and the ftclient programs. The final version of your
*    programs must accomplish the following tasks:
*    1. ftserver starts on Host A, and validates command-line parameters
*        (<SERVER_PORT>).
*    2. ftserver waits on <PORTNUM> for a client request.
*    3. ftclient starts on Host B, and validates any pertinent 
*        command-line parameters. (<SERVER_HOST>, <SERVER_PORT>, 
*        <COMMAND>, <FILENAME>, <DATA_PORT>, etc…)
*    4. ftserver and ftclient establish a TCP control connection 
*        on <SERVER_PORT>. (For the remainder of this description, call
*        this connection P)
*    5. ftserver waits on connection P for ftclient to send a command.
*    6. ftclient sends a command (-l (list) or -g <FILENAME> (get)) on 
*        connection P.
*    7. ftserver receives command on connection P.
*        If ftclient sent an invalid command
*            a) ftserver sends an error message to ftclient on connection
*                P, and ftclient displays the message on-screen.
*        otherwise
*            a) ftserver initiates a TCP data connection with ftclient 
*                on <DATA_PORT>. (Call this connection Q)
*            b) If ftclient has sent the -l command, ftserver sends its
*                directory to ftclient on connection Q, and ftclient 
*                displays the directory on-screen.
*            c) If ftclient has sent -g <FILENAME>, ftserver validates FILENAME, and
*                either
*                    i) sends the contents of FILENAME on connection Q. 
*                        ftclient saves the file in the current default 
*                        directory (handling "duplicate file name" error if
*                        necessary), and displays a "transfer complete" 
*                        message on-screen
*                or
*                    ii) sends an appropriate error message (“File not found”
*                        , etc.) to ftclient on connection P, and ftclient 
*                        displays the message on-screen.
*            d) ftserver closes connection Q (don’t leave open sockets!).
*    8. ftclient closes connection P (don’t leave open sockets!) and terminates.
*    9. ftserver repeats from 2 (above) until terminated by a supervisor (SIGINT).
*
*********************************************************************/
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define MAX_SIZE 65000

/******************************************************************************
* Function: struct sockaddr_in setUpAddress(char* pn, char* user)
* Description: set up the server address structure. A lot of the socket set up
*   was taken from CS344 examples given by Benjamin Brewster. Specifically it came
*   from client.c file provided in Block 4 of CS344 course. Also taken from my
*   CS344 Project 4 - OTP. Please ask for my code if you would like to reference it.
* Input: argv[1] (portNumber)
* Output: struct sockaddr_in serverAddress
******************************************************************************/
struct sockaddr_in setUpAddress(char* pn){
    int portNumber;
    int listenSocketFD;
    struct sockaddr_in serverAddress;

    //Initialize the address structure by clearing it out
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    
    //Taken from CS344 client.c file provided by Benjamin Brewster in Block 4 of course
    portNumber = atoi(pn); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    //return the address to main
    return serverAddress;
}

/******************************************************************************
* Function: struct sockaddr_in setUpDataAddress(char* pn, char* user)
* Description: set up the server address structure. A lot of the socket set up
*   was taken from CS344 examples given by Benjamin Brewster. Specifically it came
*   from client.c file provided in Block 4 of CS344 course. Also taken from my
*   CS344 Project 4 - OTP. Please ask for my code if you would like to reference it.
* Input: portNumber, ipAddr
* Output: struct sockaddr_in serverAddress
******************************************************************************/
struct sockaddr_in setUpDataAddress(char* pn, char* ipAddr){
    int portNumber;
    int listenSocketFD;
    struct sockaddr_in serverAddress;

    //Initialize the address structure by clearing it out
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    
    //Taken from CS344 client.c file provided by Benjamin Brewster in Block 4 of course
    portNumber = atoi(pn); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
    serverAddress.sin_addr.s_addr = inet_aton(ipAddr);

    //return the address to main
    return serverAddress;
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
        perror("FTSERVER: Error opening socket\n");
        exit(1);
    }

    // Use SO_REUSEADDR to Indicates that the rules used in validating addresses supplied in a bind() call should allow reuse of local addresses.
	int enable = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    // Enable the socket to begin listening
	if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to port
		fprintf(stderr, "ERROR on binding\n");
        exit(1);
	}
	if(listen(socketFD, 5)< 0){ // Flip the socket on - it can now receive up to 5 connections
        fprintf(stderr, "ERROR in listening\n");
        exit(1);
    }

    return socketFD;
}


int createDataSocket(struct sockaddr_in serverAddress){
    //create the socket
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0){
        perror("FTSERVER: Error opening socket\n");
        exit(1);
    }

    //connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        perror("FTSERVER: Error connecting to server\n");
        exit(1);
    }

    return socketFD;
}

/*******************************************************************************
* Function: int bindAndListen(int socketFD)
* Description: this function will be responsible for initializing the bind() and
*   listen() function necessary for the server to listen for the connection from
*   the client. This function will return the socketFD.
* Input: int socketFD
* Output: int socketFD
*******************************************************************************/
// int bindAndListen(int socketFD){
//     // Use SO_REUSEADDR to Indicates that the rules used in validating addresses supplied in a bind() call should allow reuse of local addresses.
// 	int enable = 1;
// 	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

//     // Enable the socket to begin listening
// 	if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to port
// 		fprintf(stderr, "ERROR on binding\n");
//         exit(1);
// 	}
// 	if(listen(socketFD, 5)< 0){ // Flip the socket on - it can now receive up to 5 connections
//         fprintf(stderr, "ERROR in listening\n");
//         exit(1);
//     }

//     return socketFD;
// }


/*******************************************************************************
* Function: int recvMessage(int socketFD)
* Description: this function will recv in the messages sent by the client side. 
*   The client will send things such as the command, data port and confirming that
*   the directory or file connents were recieved.
* Input: int socketFD;
* Output: N/A
*******************************************************************************/
void recvMessage(int socketFD, char* inMessage){    
    //initialize the inMessage each time this function is called
    memset(inMessage, '\0', sizeof(inMessage));

    //recv in the message from the client and double check that the message was
    // recv correctly.
    int check = 0;
    check = recv(socketFD, inMessage, sizeof(inMessage) - 1, 0);
    if (check < 0){
        perror("FTSERVER: Error receiving message from ftclient\n");
        exit(1);
    }
    else if (check == 0){
        printf("The connection has been closed by the ftclient\n");
    }
}

/*******************************************************************************
* Function: int sendConfirm(int socketFD)
* Description: This function will send a confirmation message to the client that
*   the message sent by the client was recieved correctly and is ready for the
*   next message to be sent to the server.
* Input: int socketFD
* Output: N/A
*******************************************************************************/
void sendConfirm(int socketFD){
    //declare variables
    char confirm[] = "OK";

    //Send confirm to client to confirm the recv of something
    int check = send(socketFD, confirm, sizeof(confirm), 0);
    if (check < 0){
        perror("FTSERVER: Error sending message to ftclient\n");
        exit(1);
    }
    if (check < strlen(confirm)){
        printf("FTSERVER: WARNING: Not all data written to socket!\n");
    }
}

/*******************************************************************************
* Function: int getDir(char* listOfFiles[], int* numOfFiles)
* Description: This function will get the current directory from the server if the
*   file. If the file is a regular file, it will be added to the list of files in
*   the directory. The number of files in the directory will also be tracked to 
*   know how many characters are to be sent to the client.
* Input: char* listOfFiles[], int* numOfFiles
* Output: int cDirSize;
*******************************************************************************/
int getDir(char* listOfFiles[], int* numOfFiles){
    //declare variables
    struct dirent* cDirectory;
    DIR* cDir;
    int cDirSize = 0;
    int i = 0;

    //open the current directory
    cDir = opendir(".");
    if (cDir == NULL){
        printf("Cannot open directory\n");
        return -1;
    }

    //Copy the directory into an array of characters
    while ((cDirectory = readdir(cDir)) != NULL){

        //Check if the file is a regular file, if it is add to array of characters
        if(cDirectory->d_type == DT_REG){
            listOfFiles[i] = cDirectory->d_name;
            cDirSize += strlen(listOfFiles[i]);
            i++;
        }
    }

    //Add the number of newline characters that need to be sent to cDirSize
    cDirSize += i - 1;
    *numOfFiles = i;

    //Close directory
    closedir(cDir);

    return cDirSize;
}


/*******************************************************************************
* Function: getFileSize(char* fileName)
* Description: using the file name from the client request, determine if the
*   file exists and if it can be opened. If it does, get the size of the file to
*   send back to the client so they know the size of the file to be sent.
* Input: char* fileName
* Output: int size;
*******************************************************************************/
int getFileSize(char* fileName){
    //Declare variables
    FILE* fptr = fopen(fileName, "r");
    
    //Check to see if the file exist or fails to open
    if (fptr == NULL){
        printf("getFileSize: Unable to open file\n");
        return -1;
    }

    int fd = fileno(fptr);
    
    //If the file was successfully opened, determine the size of the file in bytes
    struct stat st;
    if (fstat(fd, &st) < 0){
        printf("getFileSize: file size is less than 0\n");
        return -1;
    }
    int size = st.st_size;

    //close the file
    fclose(fptr);

    //Return the size of the file
    return size;
}

/*******************************************************************************
* Function: void sendFile(int socketFD, char* fileName, int fileSize)
* Description: send the file that the user requested. Keep track that the entire
*   file was sent. Will call setUpAddress and createSocket to create the connection
*   between the server and the client to send the file on.
*******************************************************************************/
int sendFile(int socketFD, char* fileName, int fileSize){
    if (fileSize < 0){
        char noFile[] = "Error: File Not Found or Do Not Have Permission To Send";
        int check = send(socketFD, noFile, sizeof(noFile), 0);
        if (check < 0){
            perror("FTSERVER: Error sending file to ftclient\n");
            exit(1);
        }
        if (check < strlen(noFile)){
            printf("FTSERVER: WARNING: Not all data written to socket!\n");
        }
        return 0;
    }
    else {
        //Declare variables
        char sendBuffer[100];
        int x;
        FILE* fptr = fopen(fileName, "r");
    
        //Check to see if the file exist or fails to open
        if (fptr == NULL){
            printf("getFileSize: Unable to open file\n");
            return -1;
        }

        //Send the contents of the file to the client
        while((x = fread(sendBuffer, 1, sizeof(sendBuffer), fptr)) > 0){
            send(socketFD, sendBuffer, x, 0);
        }

        fclose(fptr);
        return 0;
    }

}


/*******************************************************************************
* Function: void sendDir(int socketFD, char* listOfFiles[], int dirSize)
* Description: send the entire list of files in the directory. Keep track that 
*   the entire list of characters was sent. Will call setUpAddress and createSocket
*   to create the connection between the server and the client to send the directory.
*******************************************************************************/
void sendDir(int socketFD, char* listOfFiles, int dirSize){
    size_t stringLength = strlen(listOfFiles);
    size_t charWritten = 0;

    while(charWritten < stringLength){
        ssize_t check = send(socketFD, listOfFiles, stringLength, 0);
        if(check < 0){
            fprintf(stderr, "FTSERVER: ERROR writing directory to socket\n"); exit(1);
        }
        charWritten += check;
    }

}


/*******************************************************************************
* Function: int main(int argc, char* argv[])
* Description:
*    argv[1] - server port number 
*******************************************************************************/
int main(int argc, char* argv[]) {
    //Declare variables
    int socketFD;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
	socklen_t sizeOfClientInfo;


    //Check the correct number of arguments were given, if not error and exit
    if (argc != 2){
        fprintf(stderr, "Please enter in format: ./ftserver [port#]\n"); exit(1);
    }

    //Set up the server address structure
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress = setUpAddress(argv[1]);

    //create the socket and establish a connection to client
    socketFD = createSocket(serverAddress);
    
    //Keep this socket up and running until ended with a SIGINT call.
    while(1){
        //intialize variables
        char command[10];
        char dataPort[10];
        char sizeConfirm[10];
        char ipAddr[100];
        char ipSize[10];

        //Wait to accept connection
        int sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		int establishedConnectionFD = accept(socketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {
			fprintf(stderr, "ERROR on accept\n"); fflush(stdout);
		}

        //receive command request
        recvMessage(establishedConnectionFD, command);
        sendConfirm(establishedConnectionFD);

        //determine what request to perform
        if (strstr(command, "l") != NULL){
            //Initialize directory character array
            char* directoryArray[MAX_SIZE];
            memset(directoryArray, '\0', sizeof(directoryArray));

            //get the directory
            int numOfFiles;
            int dirSize = getDir(directoryArray, &numOfFiles);

            //receive the data port number
            recvMessage(establishedConnectionFD, dataPort);
            printf("dataPort: %s\n", dataPort);

            //send confirmation that data port was recv.
            sendConfirm(establishedConnectionFD);

            //get ip address from client
            recvMessage(establishedConnectionFD, ipSize);
            int sizeOfIP = atoi(ipSize);
            printf("ipSize: %d\n", sizeOfIP);

            //send confirmation that ip address was recv.
            sendConfirm(establishedConnectionFD);

            //get ip address from client
            recvMessage(establishedConnectionFD, ipAddr);

            int check = 0;
            while (check < sizeOfIP){
                int check2 = recv(establishedConnectionFD, ipAddr, sizeof(ipAddr) - 1, 0);
                if (check2 < 0){
                    perror("FTSERVER: Error receiving message from ftclient\n");
                    exit(1);
                }
                else if (check2 == 0){
                    printf("The connection has been closed by the ftclient\n");
                }
                check += check2;
            }

            printf("dataPort: %s\n", ipAddr);

            //send confirmation that ip address was recv.
            sendConfirm(establishedConnectionFD);

            //establish the data port connection
            int DPSocket;
            struct sockaddr_in clientAddressDP;
            memset((char*)&clientAddressDP, '\0', sizeof(clientAddressDP));
            printf("connection? 1\n");fflush(stdout);
            clientAddressDP = setUpDataAddress(dataPort, ipAddr);
            printf("connection? 2\n");fflush(stdout);
            DPSocket = createDataSocket(clientAddressDP);

            printf("connection!\n");fflush(stdout);

            //send size of directory
            int confirm = send(DPSocket, &dirSize, sizeof(dirSize), 0);
            if (confirm < 0){
                fprintf(stderr, "FTSERVER: Error sending the directory size"); exit(1);
            }
            
            //Confirm the size of the directory was recv by client
            recvMessage(establishedConnectionFD, sizeConfirm);

            //send the directory
            int i = 0;
            while (i < numOfFiles){
                sendDir(DPSocket, directoryArray[i], dirSize);
                i++;
            }

            //receive ack that the client got the directory
            sendConfirm(establishedConnectionFD);

            //close the data port connection
            close(DPSocket);

        }
        else if(strstr(command, "g") != NULL){
            //receive the data port number
            recvMessage(establishedConnectionFD, dataPort);
            
            //send confirmation that recv dataPort
            sendConfirm(establishedConnectionFD);

            //get ip address from client
            recvMessage(establishedConnectionFD, ipAddr);

            //send confirmation that ip address was recv.
            sendConfirm(establishedConnectionFD);

            //receive fileName
            char fileName[MAX_SIZE];
            memset(fileName, '\0', sizeof(fileName));
            recvMessage(establishedConnectionFD, fileName);
            sendConfirm(establishedConnectionFD);

            //establish the data port connection
            int DPSocket;
            struct sockaddr_in clientAddressDP;
            memset((char*)&clientAddressDP, '\0', sizeof(clientAddressDP));
            clientAddressDP = setUpDataAddress(dataPort, ipAddr);
            DPSocket = createDataSocket(clientAddressDP);

            //check to see if the file exits (send an ack saying if the file exists or not)
            int fileSize = getFileSize(fileName);

            //send size of file
            int confirm = send(DPSocket, &fileSize, sizeof(fileSize), 0);
            if (confirm < 0){
                fprintf(stderr, "FTSERVER: Error sending the directory size"); exit(1);
            }

            //Confirm the size of the file was recv by client
            recvMessage(DPSocket, sizeConfirm);

            //if the file exists, send the file contents
            int errorTest = sendFile(DPSocket, fileName, fileSize);
            if (errorTest < 0){
                fprintf(stderr, "FTSERVER: Error sending the file contents"); exit(1);
            }

            //receive ack that the client got the file contents
            sendConfirm(establishedConnectionFD);

            //close the data port connection
            close(DPSocket);

        }
        

    }

    //close the initial connection
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
* https://stackoverflow.com/questions/15739490/should-use-size-t-or-ssize-t
* https://stackoverflow.com/questions/2620146/how-do-i-return-multiple-values-from-a-function-in-c
*
* //open, read and close directory
* https://stackoverflow.com/questions/3554120/open-directory-using-c
* http://man7.org/linux/man-pages/man3/closedir.3.html
* http://man7.org/linux/man-pages/man3/opendir.3.html
* http://man7.org/linux/man-pages/man3/readdir.3.html
*
* //Sockets
* https://beej.us/guide/bgnet/html/multi/setsockoptman.html
*
* //Send and recv files
* https://stackoverflow.com/questions/11952898/c-send-and-receive-file
*
* //Getting a file size
* https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
******************************************************************************/
