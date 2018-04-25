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




/* FUNCTION PROTOTYPES */
void chomp(char *message);		// removes '\n' char at end of message

void clear_buff();				// removes remaining data left over in stdin


#endif