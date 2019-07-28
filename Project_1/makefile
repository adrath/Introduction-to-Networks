###
### @Author - Alexander Drath
### @Date - 7/27/19
### @Description: The makefile for project 1 in CS372

# Project Name
#
PROJ = CS372_Project_1_Drath_Alexander

# The Compiler: gcc for C program
#
CC = gcc

# Compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
CFLAGS = -std=c99
CFLAGS = -g
#CFLAGS = -Wall

# name of the outputfile executable:
#
OUTPUTFILE = chatClient

# name of the object file
#
OBJS = chatClient.o

# name of source files
SRCS = chatClient.c

# Compiling the object files using the compilers
#
$(OUTPUTFILE): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTPUTFILE) $(SRCS)

# Making the object files for the program 
#
${OBJS}: ${SRCS}
	${CC} ${CFLAGS} -c $(SRCS)

# Cleaning the object files and program made by the makefile
#
clean:
	$(RM) $(OBJS) *zip $(OUTPUTFILE)

# Make a zip file of all of the source files and the makefile
#
zip:
	zip $(PROJ) .zip *.c makefile README.txt
