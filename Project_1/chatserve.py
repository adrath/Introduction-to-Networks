#!/bin/python
'''
 Author: Alexander Drath
 Date: 7/24/19
 Last Updated: 7/24/19
 Course: CS372 - Intro to Networks
 Assignment: Project 1
 Filename: chatserve.py
 Description: Design and implement a simple chat system that works for one pair 
    of users, i.e., create two programs: a chat server and a chat client.  The
    final version of your programs must accomplish the following tasks:
        1. chatserve starts on host A. 
        2. chatserve on host A waits on a port (specified by command-line) for 
            a client request. 
        3. chatclient starts on host B, specifying host A's hostname and port 
            number on the command line. 
        4. chatclient on host B gets the user's "handle" by initial query (a 
            one-word name, up to 10 characters).  chatclient will display this 
            handle as a prompt on host B, and will prepend it to all messages 
            sent to host A.  e.g.,  "SteveO> Hi!!" 
        5. chatclient on host B sends an initial message to chatserve on host 
            A : PORTNUM.  This causes a connection to be established between 
            Host A and Host B.  Host A and host B are now peers, and may alternate 
            sending and receiving messages.  Responses from host A should have 
            host A’s "handle" prepended. 
        6. Host A responds to Host B, or closes the connection with the command "\quit" 
        7. Host B responds to Host A, or closes the connection with the command "\quit" 
        8. If the connection is not closed, repeat from 6. 
        9. If the connection is closed, chatserve repeats from 2 
            (until a SIGINT is received)
''' 

import sys
import socket

'''
Function: getUsername(username)
Description: get the username for the server that the user will input.
Input: username
Output: username
'''
def getUsername(username):
    while len(username) > 10 or len(username) == 0:
        username = input("Error! Please enter a username that is less than 10 characters")
    return username

'''
Function: exchangeUsernames(connection, username)
Description: establish the connection between the server and client. Receive and send the
    username to the client.
Input: connection, username
Output: clientUsername
'''
def exchangeUsernames(connection, username):
    clientUsername = connection.recv[510]
    connection.send(username)
    return clientUsername


'''
Function: sendAndRecv(connection, clientUsername, username)
Description: send and receive messages up to 500 bytes long. If the client sends \quit, the
    chat should end and return back to the main function.
Input: connection, clientUsername, username
Output: N/A
'''
def sendAndRecv(connection, clientUsername, username):
    #Continue the chat in an infinite loop until the user enters \quit.
    while 1:
        #receive the message from the client and remove the extra \n attached at the end of the message
        fromClient = connection.recv(501)[0:-1]

        #determine if the message from the client is blank
        if fromClient == "":
            print "Connection has ended, exiting chat with %s" % clientUsername
            break;

        #print message to server
        print "%s> %s" % (clientUsername, fromClient)
        
        #write a message to be sent to the client
        toClient = ""
        while len(toClient) > 500 or len(toClient) == 0:
            toClient = input("%s> " % username)
        
        #detemine if the user input a string that contains \quit, discontinue the connection with client
        if "\quit" in toClient:
            print "Connection has been terminated, exiting chat with %s" % clientUsername
            break;
        
        connection.send(toClient)


'''
Function: if __name__ == "__main__":
Description: Refer to file description above.
'''
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Please enter in format: python chatServer.py [port]"
        exit(1)

    #Get the port number that the user input
    portNumber = sys.argv[1]

    #Create a TCP socket object
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    #Bind to the port
    serverSocket.bind((",int(portNumber)))

    #Look at up to 1 request
    serverSocket.listen(1)

    #Get the username from the user
    username = ""
    username = getUsername(username)

    #Run an infinite loop so that the server does not shut down unless SIGINT is received.
    while 1:
        #establish a new socket object usable to send and receive data on the connection and the address
        #   bound to the socket on the other end of the connection.
        connection, addr = serverSocket.accept()
        print "Connected on address %s" % str(addr)

        #Get the clients username
        clientUsername = exchangeUsernames(connection, username)

        #Begin sending and receiving messages with the client
        sendAndRecv(connection, clientUsername, username)

        connection.close()


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
'''
