/*
 * chat_library.c
 *
 * Author: Noah Williamson
 * Course: CS371
 * Project 2
 * 
 * implements chat_library header file
 */

#include "chat_library.h"

#include <string.h>

void chomp(char *text) {
  
  int length = strlen(text) - 1;
  
  if (text[length] == '\n')		// get rid of the trailing newline
      text[length] = '\0';
}

void clear_buff() {
  int c;
  while( (c = getchar() ) != '\n' && c != EOF )
  	/* discards content in stdin buffer*/;
}
