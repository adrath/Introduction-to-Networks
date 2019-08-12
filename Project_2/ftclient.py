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
         <COMMAND>, <FILENAME>, <DATA_PORT>, etc…)
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
                     ii) sends an appropriate error message (“File not found”
                         , etc.) to ftclient on connection P, and ftclient 
                         displays the message on-screen.
             d) ftserver closes connection Q (don’t leave open sockets!).
     8. ftclient closes connection P (don’t leave open sockets!) and terminates.
     9. ftserver repeats from 2 (above) until terminated by a supervisor (SIGINT).

'''


'''
Function: createSocket()
Description: set up a socket to connect to ftserver.

Input:
Output:
'''


'''
Function: 
Description:

Input:
Output:
'''


'''
Function: 
Description:

Input:
Output:
'''


'''
Function: 
Description:

Input:
Output:
'''




'''
Function: if __name__ == "__main__":
Description:
'''
if __name__ == "__main__":
    #validate user input length
    inputLength = strlen(argv)
    if (inputLength < 5 or inputLength > 6):
        print("Please enter in format: python ftclient.py [server host] [server port] [command] [filename] [data port]\n")
        exit(1)

    #initialize the variables from user input
    serverHost = sys.argv[1]
    serverPort = sys.argv[2]
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

    #Create the initial TCP socket object (Connection P)
    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    #Bind to the port
    serverSocket.bind(('',int(serverPort)))

    #Look at up to 1 request
    serverSocket.listen(1)

    #establish a new socket object usable to send and receive data on the connection and the address
    #   bound to the socket on the other end of the connection.
    initConnP, addr = serverSocket.accept()
    print "Connected on address %s" % str(addr)

    #If getting the directory from the server
    if (commandID == "l"):
        #send commandID of "l" to the server
        initConnP.send(commandID)

        #wait for ack that received the request for the directory
        confirmation = initConnP.recv(3)[0:-1]
        if (confirmation != "OK"){
            print "FTCLIENT: ERROR sending or receiving command\n"
            print "Confirmation = %s\n" % str(confirmation)
            exit(1)
        }

        #set up the new socket on data port (connection Q)
        #Create the initial TCP socket object (Connection P)
        clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        #Bind to the port
        clientSocket.bind(('',int(serverPort)))

        #Look at up to 1 request
        clientSocket.listen(1)

        #establish a new socket object usable to send and receive data on the connection and the address
        #   bound to the socket on the other end of the connection.
        dataConnQ, addr = clientSocket.accept()
        print "Connected on address %s" % str(addr)

        #send the data port number to server on connection P
        initConnP.send(dataPort)

        #receive confirmation that data port was received
        confirmation = initConnP.recv(3)[0:-1]
        if (confirmation != "OK"){
            print "FTCLIENT: ERROR sending or receiving data port\n"
            print "Confirmation = %s\n" % str(confirmation)
            exit(1)
        }

        #receive the size of the directory from the server on connection Q
        int(dirSize) = dataConnQ.recv(7)[0:-1]
        
        #determine if the message from the client is blank
        if fromClient == "":
            print "Connection has ended, exiting chat with %s" % clientUsername
            print "dirSize = %d\n" % int(dirSize)
            exit(1)

        #receive the directory from the server on connection Q
        x = 0
        while (x < dirSize):
            dirFromServer = dataConnQ.recv(dirSize)[0:-1]
            x += strlen(dirFromServer)
            print "%s" % dirFromServer

        #send confirmation that the directory was received
        dataConnQ.send("OK")

        dataConnQ.close()


    #if getting a file from the server
    #elif (commandID == "g"):
        #send commandID of "g" to the server

        #wait for ack that received the request for the filename

        #set up the new socket on data port (connection Q)

        #send the data port number to server on connection P

        #receive confirmation that data port was received

        #receive the size of the directory from the server on connection Q

        #receive the file contents from the server on connection Q

        #place file contents from the server into a file

        #print successfully placed into file

    
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
'''
