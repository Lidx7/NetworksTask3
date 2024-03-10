# Define the compiler
CC = gcc

# Define the flags for compiling C files
CFLAGS = -Wall -g

all: TCP_Receiver TCP_Sender

TCP_Receiver: TCP_Receiver.c 
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.c

TCP_Sender: TCP_Sender.c
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.c

clean:
	rm TCP_Receiver TCP_Sender