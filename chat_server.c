/*
 * chat_server.c
 *
 * Author: Noah Williamson
 * Course: CS371
 * Project 2
 *
 * Simple terminal-based chat room server in C. We can specify the port via 
 * command line arguments. 
 * Note: The client assumes connection to localhost and at port 8888!!!!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>		// library for sockets + their primitives
#include <netinet/in.h>
#include <pthread.h>		// library for threading

#include "chat_library.h"

/* CONSTANTS */
#define MAX_CLIENTS 3		// max number of clients is 3

/* FUNCTION PROTOTYPES */
void start_server(connection_info *server_info, int port);				// init and start up the chat server
void send_message(connection_info clients[], int sender, char *message_text);	// to handle sending messages
void send_connect_message(connection_info *clients, int sender);			// send msg when user connects 
void send_disconnect_message(connection_info *clients, char *username);		// send msg when user disconnects
void send_too_full_message(int socket);			// for when the chat is too full for adding a new client!
void stop_server(connection_info connection[]);							// quit the chat server
void handle_client_message(connection_info clients[], int sender);		// client message handler function
int construct_fd_set(fd_set *set, connection_info *server_info, connection_info clients[]); // constructs list of connected clients!
void handle_new_connection(connection_info *server_info, connection_info clients[]); 	// handles new connection to the server
void handle_input(connection_info clients[]);		// handles user input (i.e. for quitting, etc.)


/* MAIN FUNCTION */
int main(int argc, char **argv) {
	fd_set file_descriptors;

	connection_info server_info;
	connection_info clients[MAX_CLIENTS];

	for (int i = 0; i < MAX_CLIENTS; ++i) {
    	clients[i].socket = 0;
	}

	if (argc != 2) {
    	fprintf(stderr, "Usage: %s <port>\n", argv[0]);		// we should specify what port to serve on
    	exit(1);
	}

	start_server(&server_info, atoi(argv[1]));		// get server started

	while(true) {		// start server loop
    	int max_fd = construct_fd_set(&file_descriptors, &server_info, clients);		// create file descriptor set

    	if(select(max_fd+1, &file_descriptors, NULL, NULL, NULL) < 0) {		// monitor file descriptor sets
      		perror("Select Failed");
      		stop_server(clients);
    	}

    	if(FD_ISSET(STDIN_FILENO, &file_descriptors)) {
      		handle_input(clients);			// handle user input
    	}

    	if(FD_ISSET(server_info.socket, &file_descriptors)) {
    		handle_new_connection(&server_info, clients);		// handle a new connection
    	}

    	for(int i = 0; i < MAX_CLIENTS; ++i) {
    		if(clients[i].socket > 0 && 
    				FD_ISSET(clients[i].socket, &file_descriptors))
        		handle_client_message(clients, i);
    	}
	}

	return 0;
}

/*
 * function to startup server
 *
 */
void start_server(connection_info *server_info, int port) {
  	if((server_info->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	 // create socket
    	perror("Failed to create socket");	
    	exit(1);
  	}

  	server_info->address.sin_family = AF_INET;
  	server_info->address.sin_addr.s_addr = INADDR_ANY;
  	server_info->address.sin_port = htons(port);		// set up server address struct

  	// attempt to bind socket
  	if(bind(server_info->socket, (struct sockaddr *)&server_info->address, sizeof(server_info->address)) < 0) {
    	perror("Binding failed");
    	exit(1);
  	}

  	const int optVal = 1;
  	const socklen_t optLen = sizeof(optVal);
  	if(setsockopt(server_info->socket, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen) < 0) {
    	perror("Set socket option failed");
    	exit(1);
  	}


  	if(listen(server_info->socket, 3) < 0) {		// start to listen for connections
    	perror("Listen failed");
    	exit(1);
  	}

  printf("Waiting for incoming connections...\n");	// wait for connections
}


/*
 * forward messages to all clients
 *
 */
void send_message(connection_info clients[], int sender, char *message_text) {
  	message msg;
  	msg.type = PUBLIC_MESSAGE;		// set message type to send to all clients
  	
  	strncpy(msg.username, clients[sender].username, 20);
  	strncpy(msg.data, message_text, 256);				// copy message test
  	
  	int i = 0;
  	for(i = 0; i < MAX_CLIENTS; ++i) {		// forward message to all clients
    	if(i != sender && clients[i].socket != 0) {
      		if(send(clients[i].socket, &msg, sizeof(msg), 0) < 0) {
          		perror("Message forwarding failed.");
          		exit(1);
      		}
    	}
  	}
}

/*
 * function to handle sending a welcome msg to new connected clients
 *
 */
void send_connect_message(connection_info *clients, int sender) {
  	message msg;
  	msg.type = CONNECT;
  
  	strncpy(msg.username, clients[sender].username, 21);
  
  	int i = 0;
  	for(i = 0; i < MAX_CLIENTS; ++i) {
    	if(clients[i].socket != 0) {
    		if(i == sender) {
        		msg.type = SUCCESS;
        	
        		if(send(clients[i].socket, &msg, sizeof(msg), 0) < 0) {
            		perror("Send failed");
            		exit(1);
        		}
      		}
      		else {
        		if(send(clients[i].socket, &msg, sizeof(msg), 0) < 0) {
            		perror("Send failed");
            		exit(1);
        		}
      		}
    	}
  	}
}

/*
 * function to send log out message
 *
 */
void send_disconnect_message(connection_info *clients, char *username) {
	message msg;
  	msg.type = DISCONNECT;
  	strncpy(msg.username, username, 21);
  	
  	int i = 0;
  	for(i = 0; i < MAX_CLIENTS; ++i) {		// send log out message to all clients
    	if(clients[i].socket != 0) {
      		if(send(clients[i].socket, &msg, sizeof(msg), 0) < 0) {
          		perror("Send failed");
          		exit(1);
      		}
    	}
  	}
}

/*
 * function to send message stating the chat is at max capacity
 *
 */
 void send_too_full_message(int socket) {
  	message too_full_message;
  	too_full_message.type = TOO_FULL;		// send message that chat is too full!

  	if(send(socket, &too_full_message, sizeof(too_full_message), 0) < 0) {
    	perror("Send failed");
      	exit(1);
  	}

  	close(socket);		// close connection
}

/*
 * function to quit the server and close all socket connections
 *
 */
void stop_server(connection_info connection[]) {
  	int i;
  	for(i = 0; i < MAX_CLIENTS; i++)
    	close(connection[i].socket);

  	exit(0);
}

/*
 * handle the message sent from client based on what type of message it is
 *
 */
void handle_client_message(connection_info clients[], int sender) {
  	int read_size;
  	message msg;

  	if( (read_size = recv(clients[sender].socket, &msg, sizeof(message), 0)) == 0 ) {
    	printf("User disconnected: %s.\n", clients[sender].username);
    	close(clients[sender].socket);
    	clients[sender].socket = 0;
    	send_disconnect_message(clients, clients[sender].username);

  	} 
  	else {						// check msg type
    	switch(msg.type) {
    		case PUBLIC_MESSAGE:
        		send_message(clients, sender, msg.data);
      			break;

      		case SET_USERNAME: ;
        		int i;
        		for(i = 0; i < MAX_CLIENTS; i++) {
          			if(clients[i].socket != 0 && !strcmp(clients[i].username, msg.username) ) {
            			close(clients[sender].socket);
            			clients[sender].socket = 0;
            			return;
          			}
        		}

        		strcpy(clients[sender].username, msg.username);
        		printf("User connected: %s\n", clients[sender].username);
        		send_connect_message(clients, sender);
      			break;

      		default:
        		fprintf(stderr, "Message type error!\n");
      			break;
    	}
  	}
}

/*
 * handle the construction sent of array of file descriptors
 * returns max_fd
 */
int construct_fd_set(fd_set *set, connection_info *server_info, connection_info clients[]) {
  	FD_ZERO(set);
  	FD_SET(STDIN_FILENO, set);
  	FD_SET(server_info->socket, set);

  	int max_fd = server_info->socket;
  	
  	int i;
  	for(i = 0; i < MAX_CLIENTS; ++i) {
    	if(clients[i].socket > 0) {
      		FD_SET(clients[i].socket, set);
      		if(clients[i].socket > max_fd) {
        		max_fd = clients[i].socket;
      		}
    	}
  	}
  
  return max_fd;
}

/*
 * function to handle a new connection the server
 *
 */
 void handle_new_connection(connection_info *server_info, connection_info clients[]) {
  	int new_socket;
  	int address_len;
  
  	// create new socket for connection
  	new_socket = accept(server_info->socket, (struct sockaddr*)&server_info->address, (socklen_t*)&address_len);

  	if (new_socket < 0) {		// check to make sure we actually did create this new socket!
    	perror("Accept Failed");
    	exit(1);
  	}

  	int i;
  	for(i = 0; i < MAX_CLIENTS; ++i) {
    	if(clients[i].socket == 0) {
      		clients[i].socket = new_socket;
      		break;

    	} 
    	else if (i == MAX_CLIENTS -1) {		// check to make sure there can be a new client!
      		send_too_full_message(new_socket);		// send error (chat too full) message
    	}
  	}
}

/*
 * function to user input (basically just to quit the server without using Control+C)
 *
 */
void handle_input(connection_info clients[]) {
  	char input[255];
  	fgets(input, sizeof(input), stdin);
  	chomp(input);		// remove \n if applicable

  	if(input[0] == 'q') {		// quit server if 'q' is input
    	stop_server(clients);
  	}
}
