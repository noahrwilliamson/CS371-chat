/*
 * chat_library.h
 *
 * Author: Noah Williamson
 * Course: CS371
 * Project 2
 * 
 * chat_library header file
 */

#ifndef CHAT_LIBRARY_H_
#define CHAT_LIBRARY_H_

#include <stdio.h>
#include <sys/socket.h>		// library for sockets + their primitives
#include <sys/types.h>
#include <netinet/in.h>


/* DATA STRUCTURES */

typedef enum{
  CONNECT,
  DISCONNECT,
  SET_USERNAME,
  PUBLIC_MESSAGE,
  TOO_FULL,
  USERNAME_ERROR,
  SUCCESS,
  ERROR
} msg_type;					// enumeration for different types of messages the server can send out

typedef struct{
  msg_type type;
  char username[21];
  char data[256];
} message;						// structure for a message

typedef struct connection_info{
  int socket;
  struct sockaddr_in address;
  char username[20];
} connection_info;			// structure to hold client connection information


/* FUNCTION PROTOTYPES */
void chomp(char *message);		// removes '\n' char at end of message

void clear_buff();				// removes remaining data left over in stdin


#endif