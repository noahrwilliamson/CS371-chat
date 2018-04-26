/*
 * chat_client.c
 *
 * Author: Noah Williamson
 * Course: CS371
 * Project 2
 *
 * We assume connection to localhost at port 8888 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>		// library for sockets + their primitives
#include <arpa/inet.h>		// library w/ definitions for internet operations

#include "chat_library.h"

/* FUNCTION PROTOTYPES */
void get_username(char *name);
void set_username(connection_info *connection);
void client_quit(connection_info *connection);		// handle closing connection when a user logs off
void connect_server(connection_info *connection, char *address, char *port);		// handles initiating connection to server
void handle_input(connection_info *connection);		// handler function for user input
void handle_server_message(connection_info *connection); 	// handler function for each type of server msg


/* MAIN FUNCTION */
int main(int argc, char **argv) {
	connection_info connection;
  	fd_set file_descriptors;

  	connect_server(&connection, " ", " ");		// initiate connection to server

  	while(true) {		// communication loop for connection with server
    	
    	FD_ZERO(&file_descriptors);
    	FD_SET(STDIN_FILENO, &file_descriptors);
    	FD_SET(connection.socket, &file_descriptors);
    	fflush(stdin);

    	if(select(connection.socket + 1, &file_descriptors, NULL, NULL, NULL) < 0) {
      		perror("Select failed.");
      		exit(1);
    	}

    	if(FD_ISSET(STDIN_FILENO, &file_descriptors)) {
      		handle_input(&connection);
    	}

    	if(FD_ISSET(connection.socket, &file_descriptors)) {
      		handle_server_message(&connection);
    	}
  	}
	
	close(connection.socket);
	return 0;
}

/*
 * function handles getting user input for their screen name
 *
 */
void get_username(char *name) {
  
	while(true) {
    	printf("Enter a username: ");
    	fflush(stdout);
    	memset(name, 0, 1000);
    	fgets(name, 22, stdin);
    	chomp(name);

    	if(strlen(name) > 20) {
      		puts("Username must be 20 characters or less.");
    	}
		else
    		break;
  	}
}

/*
 * function handles sending the username to the server
 *
 */
void set_username(connection_info *connection) {
  	message msg;
  	msg.type = SET_USERNAME;
  
  	strncpy(msg.username, connection->username, 20);

  	if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0) {
    	perror("Send failed");
    	exit(1);
  	}
}

/*
 * function to handle quitting the client and closing connection to the server
 *
 */
void stop_client(connection_info *connection) {
  	close(connection->socket);
  	exit(0);
}

/*
 * function to handle initiating connection with the server
 *
 */
void connect_server(connection_info *connection, char *address, char *port) {

  	while(true) {
    	get_username(connection->username);		// get username

    	if ((connection->socket = socket(AF_INET, SOCK_STREAM , 0)) < 0)	// create socket, check for err
        	perror("Could not create socket");
    

    	connection->address.sin_addr.s_addr = inet_addr("127.0.0.1");
    	connection->address.sin_family = AF_INET;
    	connection->address.sin_port = htons(8888);		// set up connection locally and to port 8888

    	// attempt to connect to the server
    	if (connect(connection->socket, (struct sockaddr *)&connection->address , sizeof(connection->address)) < 0) {
        	perror("Connect failed.");
        	exit(1);
    	}

    	set_username(connection);	// send username

    	message msg;
    	ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
    	
    	if(recv_val < 0) {
        	perror("recv failed");
        	exit(1);

    	}
    	else if(recv_val == 0) {
      		close(connection->socket);
      		printf("The username \"%s\" is taken, please try another name.\n", connection->username);	// send back for a valid username
      		continue;
    	}

    	break;
  	}


  	puts("Successfully connected to chat server.");
  	puts("Type /q or /quit to log off. /help for help.");	// show success message
}

/*
 * function to handle input from user via stdin
 *
 */
void handle_input(connection_info *connection) {
  	char input[255];
  	fgets(input, 255, stdin);
  	chomp(input);

  	if( !strcmp(input, "/q") || !strcmp(input, "/quit") ) {		// quit if '/q' or '/quit'
    	stop_client(connection);
  	}
  	else if(!strcmp(input, "/h") || !strcmp(input, "/help") ) {	// show help message
    	puts("/q or /quit: Logs out of the chat.");
    	puts("/h or /help: Displays this info list.");
  	}
  	else {		// otherwise its a message to send
   		message msg;
    	msg.type = PUBLIC_MESSAGE;
    	strncpy(msg.username, connection->username, 20);

    	if(strlen(input) == 0)
        	return;

    	strncpy(msg.data, input, 255);

    	if(send(connection->socket, &msg, sizeof(message), 0) < 0) {		// send msg
        	perror("Send failed");
        	exit(1);
    	}
  	}

}

/*
 * function to handle message received from the chat server
 *
 */
void handle_server_message(connection_info *connection) {
  	message msg;

  	ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);		// get reply from server
  	if(recv_val < 0) {
    	perror("recv failed");
      	exit(1);

  	}
  	else if(recv_val == 0) {
    	close(connection->socket);
    	puts("Disconnected from server.");
    	exit(0);
  	}

  	switch(msg.type) {		// check message type to determine what we need to do

    case CONNECT:
      	printf("%s has joined chat.\n", msg.username);
    	break;

    case DISCONNECT:
      	printf("%s has logged off.\n", msg.username);
    	break;

    case SET_USERNAME:
    	break;

    case PUBLIC_MESSAGE:		// show message
      	printf("%s: %s\n", msg.username, msg.data);
    	break;

    case TOO_FULL:		// chat is full 
      	fprintf(stderr, "Server is connected to max number of clients.\n");
      	exit(0);
    	break;

    default:
      	fprintf(stderr, "Unknown message type received.\n");
    	break;
  }
}
