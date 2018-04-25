/*
 * chat_server.c
 *
 * Author: Noah Williamson
 * Course: CS371
 * Project 2
 *
 * Simple terminal-based chat room server in C.
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

/* CONSTANTS */
#define MAXCLIENTS 3		// max number of clients is 3

/* FUNCTION PROTOTYPES */
void start_server();


int main(int argc, char **argv) {

  return 0;
}

