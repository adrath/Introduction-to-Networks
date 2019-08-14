'''
Author: Alexander Drath
Course: CS372 - Intro to Networks
Assignment: Project 2
Filename: ftclient.py
Date Created: 8/5/19
Last Updated: 8/11/19

Description:
     Design and implement a simple file transfer system, i.e., create 
     a file transfer server and a file transfer client. Write the 
     ftserver and the ftclient programs. The final version of your
     programs must accomplish the following tasks:
     1. ftserver starts on Host A, and validates command-line parameters
         (<SERVER_PORT>).
     2. ftserver waits on <PORTNUM> for a client request.
     3. ftclient starts on Host B, and validates any pertinent 
         command-line parameters. (<SERVER_HOST>, <SERVER_PORT>, 
         <COMMAND>, <FILENAME>, <DATA_PORT>, etc...)
     4. ftserver and ftclient establish a TCP control connection 
         on <SERVER_PORT>. (For the remainder of this description, call
         this connection P)
     5. ftserver waits on connection P for ftclient to send a command.
     6. ftclient sends a command (-l (list) or -g <FILENAME> (get)) on 
         connection P.
     7. ftserver receives command on connection P.
         If ftclient sent an invalid command
             a) ftserver sends an error message to ftclient on connection
                 P, and ftclient displays the message on-screen.
         otherwise
             a) ftserver initiates a TCP data connection with ftclient 
                 on <DATA_PORT>. (Call this connection Q)
             b) If ftclient has sent the -l command, ftserver sends its
                 directory to ftclient on connection Q, and ftclient 
                 displays the directory on-screen.
             c) If ftclient has sent -g <FILENAME>, ftserver validates FILENAME, and
                 either
                     i) sends the contents of FILENAME on connection Q. 
                         ftclient saves the file in the current default 
                         directory (handling "duplicate file name" error if
                         necessary), and displays a "transfer complete" 
                         message on-screen
                 or
                     ii) sends an appropriate error message ("File not found"
                         , etc.) to ftclient on connection P, and ftclient 
                         displays the message on-screen.
             d) ftserver closes connection Q (don't leave open sockets!).
     8. ftclient closes connection P (don't leave open sockets!) and terminates.
     9. ftserver repeats from 2 (above) until terminated by a supervisor (SIGINT).

'''
from socket import AF_INET, SOCK_STREAM, socket, SOCK_DGRAM
import sys
from os import path
from struct import *
from time import sleep

'''
Function: def getConfirm(sockFD)
Description: receive confirmation from server
Input: socket
Output: N/A
'''
def getConfirm(sockFD):
    confirmation = sockFD.recv(3)[0:-1]
    if (confirmation != "OK"):
        print "FTCLIENT: ERROR sending or receiving command\n"
        print "Confirmation = %s\n" % str(confirmation)
        exit(1)


'''
Function: if __name__ == "__main__":
Description:
'''
if __name__ == "__main__":
    #validate user input length
    inputLength = len(sys.argv)
    if (inputLength < 5 or inputLength > 6):
        print("Please enter in format: python ftclient.py [server host] [server port] [command] [filename] [data port]\n")
        exit(1)

    #initialize the variables from user input
    serverHost = sys.argv[1]
    serverPort = int(sys.argv[2])
    command = sys.argv[3]
    commandID = "f"

    #assign dataport and if necessary, the filename
    if (inputLength == 5):
        dataPort = int(sys.argv[4])
    elif (inputLength == 6):
        fileName = sys.argv[4]
        dataPort = int(sys.argv[5])

    #Make sure that the serverPort and the dataPort are within the correct port range
    if (serverPort > 65535 or dataPort > 65535 or serverPort < 1024 or dataPort < 1024):
        print("The server port and data port must be between 1024-65535\n")
        exit(1)

    #determine if the command is a valid request or -l or -g. If so, assign the commandID value
    if (command == "-l"):
        commandID = "l"
    elif (command == "-g"):
        commandID = "g"
    else:
        print("Commands must be either -l or -g\n")
        exit(1)

    #Create the initial TCP socket object (Connection P) and connect
    initConnP = socket(AF_INET, SOCK_STREAM)
    initConnP.connect((serverHost,serverPort))

    #get IP address
    IPSock = socket(AF_INET, SOCK_DGRAM)     
    IPSock.connect(("8.8.8.8", 80))
    IPAddr = IPSock.getsockname()[0]

    #If getting the directory from the server
    if (commandID == "l"):
        #send commandID of "l" to the server and wait for ack that received the request for the directory
        initConnP.send("l")
        getConfirm(initConnP)

        #send the data port number to server on connection P and receive confirmation that data port was received
        initConnP.send(str(dataPort))
        getConfirm(initConnP)

        #send the data port number to server on connection P and receive confirmation that data port was received
        initConnP.send(str(IPAddr))
        getConfirm(initConnP)

        #set up the new socket on data port (connection Q)
        clientSocket = socket(AF_INET, SOCK_STREAM)

        #Bind to the port
        clientSocket.bind(('',int(dataPort)))

        #Look at up to 1 request
        clientSocket.listen(1)

        #establish a new socket object usable to send and receive data on the connection and the address
        #   bound to the socket on the other end of the connection.
        dataConnQ, addr = clientSocket.accept()
        print "Connected on address %s" % str(addr)

        #confirm that the connection was established
        dataConnQ.send("OK")

        #receive the directory from the server on connection Q
        dirFromServer = dataConnQ.recv(100)       
        while "@" not in dirFromServer and dirFromServer != "":
        #     print "%s" % dirFromServer
            dirFromServer = dataConnQ.recv(100)

        newDirFromServer = dirFromServer.replace("@", "")
        print(newDirFromServer.replace(",", "\n"))

        #send confirmation that the directory was received
        dataConnQ.send("OK")

        dataConnQ.close()


    #if getting a file from the server
    elif (commandID == "g"):
        #send commandID of "l" to the server and wait for ack that received the request for the directory
        initConnP.send("g")
        getConfirm(initConnP)

        #send the data port number to server on connection P and receive confirmation that data port was received
        initConnP.send(str(dataPort))
        getConfirm(initConnP)

        #send the fileName to server on connection P and receive confirmation that fileName was received
        initConnP.send(str(fileName))
        getConfirm(initConnP)

        #send the data port number to server on connection P and receive confirmation that data port was received
        initConnP.send(str(IPAddr))
        getConfirm(initConnP)

        #set up the new socket on data port (connection Q)
        clientSocket = socket(AF_INET, SOCK_STREAM)

        #Bind to the port
        clientSocket.bind(('',int(dataPort)))

        #Look at up to 1 request
        clientSocket.listen(1)

        #establish a new socket object usable to send and receive data on the connection and the address
        #   bound to the socket on the other end of the connection.
        dataConnQ, addr = clientSocket.accept()
        print "Connected on address %s" % str(addr)

        #confirm that the connection was established
        dataConnQ.send("OK")
        
        #receive the size of the directory from the server on connection Q
        fileSize = dataConnQ.recv(7)[0:-1]
        print "fileSize: %s\n" % fileSize

        #determine if the message from the client is blank
        if fileSize == "" or fileSize == "0":
            print "Connection has ended, exiting chat with %s" % clientUsername
            print "fileSize = %d\n" % int(fileSize)
            exit(1)

        #confirm that the connection was established
        dataConnQ.send("OK")

        #receive the file contents from the server on connection Q
        x = 0
        fileContents = ""
        while (x < fileSize):
            fileFromServer = dataConnQ.recv(fileSize)[0:-1]
            x += len(fileFromServer)
            fileContents += fileFromServer

        #place file contents from the server into a file
        print(fileContents)

        #print successfully placed into file
        print "transfer complete"

    
    #close connection Q, then close connection P
    initConnP.close()

'''
Sources:
https://docs.python.org/release/2.6.5/library/internet.html
https://docs.python.org/release/2.6.5/library/socketserver.html
https://docs.python.org/3/howto/sockets.html
https://realpython.com/python-sockets/
https://www.geeksforgeeks.org/socket-programming-python/
https://stackoverflow.com/questions/24872125/socket-import-does-not-work-from-does-whats-the-deal
https://stackoverflow.com/questions/15869158/python-socket-listening
https://www.bogotobogo.com/python/python_network_programming_server_client.php
https://www.pythonforbeginners.com/files/reading-and-writing-files-in-python
https://www.geeksforgeeks.org/python-program-find-ip-address/
https://stackoverflow.com/questions/3559559/how-to-delete-a-character-from-a-string-using-python

'''
