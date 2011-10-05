/*
 * chessdin.h
 *
 *  Created on: Sep 9, 2010
 *      Author: bluebot
 */

#ifndef CHESSDIN_H_
#define CHESSDIN_H_

#include <argp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include <somatic.h>
#include <ach.h>
#include <somatic/util.h>
#include <somatic.pb-c.h>

#define BOARDSTATE_SIZE 128
#define MOVESTRING_SIZE 10
static const char *craftyi_chan_name = "craftygeneric-in";
Somatic__Crafty* craftyi_message;
ach_channel_t *craftyi_chan;

char* craftyi_boardstate;
char* craftyi_move;

// Initialize ACH Channel and all data
int ACHInit_In() {
  // Channels
  int r = 0;
  printf("ACH: Creating Channel\n");
  ach_create_attr_t attr;
  ach_create_attr_init(&attr);
  r = ach_create((char*)craftyi_chan_name, 100, BOARDSTATE_SIZE+MOVESTRING_SIZE, &attr);
  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));

  printf("ACH: Opening Channel\n");
  craftyi_chan = (ach_channel_t*) malloc (sizeof(ach_channel_t));
  if(craftyi_chan == NULL) return 0;
  r = ach_open(craftyi_chan, craftyi_chan_name, NULL);
  if(r != ACH_OK) {
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  ach_chmod(craftyi_chan, SOMATIC_CHANNEL_MODE );

  r = ach_flush(craftyi_chan);

  if(r != ACH_OK) {
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  // Data
  craftyi_message = (Somatic__Crafty*)malloc(sizeof(Somatic__Crafty));
  if(craftyi_message == NULL) return 0;
  somatic__crafty__init(craftyi_message);
  //craftyi_message->boardstate.data = malloc(BOARDSTATE_SIZE );
  //craftyi_message->has_boardstate = 1;
  //craftyi_message->boardstate.len = BOARDSTATE_SIZE ;
  craftyi_message->move = (char*)malloc(MOVESTRING_SIZE);

  printf("ACH: Info\n"
		 "ACH: Name: %s\n"
		 "ACH: Size: %d\n", craftyi_chan_name, somatic__crafty__get_packed_size(craftyi_message));

  // Output data
  craftyi_boardstate = (char*) malloc(128);
  craftyi_move = (char*) malloc (10);
  if(craftyi_boardstate == NULL || craftyi_move == NULL) return 0;

  return 1;
}

int ACHRead_In(char* buffer) {
  int r = 0;
  struct timespec abstime = aa_tm_now();
  Somatic__Crafty *msg =
		  SOMATIC_WAIT_LAST_UNPACK(r, somatic__crafty,  &protobuf_c_system_allocator, BOARDSTATE_SIZE+MOVESTRING_SIZE , craftyi_chan, &abstime);

  if(r == ACH_OK) {
	  memcpy(buffer, msg->move, MOVESTRING_SIZE);
	  somatic__crafty__free_unpacked(msg, &protobuf_c_system_allocator);
  }
  return MOVESTRING_SIZE;

}

void ACHClose_In() {
  printf("ACH: Closing ACH\n");
  int r = ach_close(craftyi_chan);
  if(r != ACH_OK)
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
  //free(craftyi_message->boardstate.data);
  free(craftyi_message->move);
  free(craftyi_message);
}


#endif /* CHESSDIN_H_ */
