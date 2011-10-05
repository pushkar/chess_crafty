/* -*- mode: C; c-basic-offset: 2  -*- */
#ifndef CHESSD_H_
#define CHESSD_H_

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
#include "tokend.h"

#define BOARDSTATE_SIZE 128
#define MOVESTRING_SIZE 10
static const char *craftyo_chan_name = "craftygeneric-out";
Somatic__Crafty* craftyo_message;
ach_channel_t *craftyo_chan;

char* craftyo_boardstate;
char* craftyo_move;

// Initialize ACH Channel and all data
int ACHInit() {
  // Channels
  int r = 0;
  printf("ACH: Creating Channel\n");
  ach_create_attr_t attr;
  ach_create_attr_init(&attr);
  r = ach_create((char*)craftyo_chan_name, 100, BOARDSTATE_SIZE+MOVESTRING_SIZE, &attr);
  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));

  printf("ACH: Opening Channel\n");
  craftyo_chan = (ach_channel_t*) malloc (sizeof(ach_channel_t));
  if(craftyo_chan == NULL) return 0;
  r = ach_open(craftyo_chan, craftyo_chan_name, NULL);
  if(r != ACH_OK) {
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  ach_chmod(craftyo_chan, SOMATIC_CHANNEL_MODE );

  r = ach_flush(craftyo_chan);

  if(r != ACH_OK) {
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  // Data
  craftyo_message = (Somatic__Crafty*)malloc(sizeof(Somatic__Crafty));
  if(craftyo_message == NULL) return 0;
  somatic__crafty__init(craftyo_message);
//  craftyo_message->boardstate.data = malloc(128);
//  craftyo_message->has_boardstate = 1;
//  craftyo_message->boardstate.len = 128;
  craftyo_message->move = (char*)malloc(10);

  printf("ACH: Info\n"
		 "ACH: Name: %s\n"
		 "ACH: Size: %d\n", craftyo_chan_name, somatic__crafty__get_packed_size(craftyo_message));

  // Output data
  craftyo_boardstate = (char*) malloc(128);
  craftyo_move = (char*) malloc (10);
  if(craftyo_boardstate == NULL || craftyo_move == NULL) return 0;

  printf("\n");
  if(!ACHTokenInit()) return 0;

  return 1;
}

// For debugging
void ACHPrint(char* move, char* position) {
	int i = 0, j = 0;
	printf("Move: %s\n", move);
	printf("State:\n");
	for(i = 0; i < 16; i++) {
		if(i == 8) printf("\n");
		for(j = 0; j < 8; j++) {
			printf("%4d ", position[i*8+j]);
		}
		printf("\n");
	}
	printf("\n\n");
}

void ACHRecordBoardStatePrev(char* position) {
	memcpy(craftyo_boardstate, position, 64);
}

void ACHRecordBoardStateNext(char* position) {
	memcpy(craftyo_boardstate+64, position, 64);
}

void ACHRecordMove(char* move) {
	memcpy(craftyo_move, move, 10);
}

void ACHWrite() {
  memcpy(craftyo_message->move, craftyo_move, 10);
//  memcpy(craftyo_message->boardstate.data, craftyo_boardstate, 128);
//  craftyo_message->has_boardstate = 1;
//  craftyo_message->boardstate.len = 128;
  int r = SOMATIC_PACK_SEND(craftyo_chan, somatic__crafty, craftyo_message);
  if(r != ACH_OK)
	  printf("ACH: Writing to channel returned %s\n", ach_result_to_string((ach_status_t)r));
  ACHTokenWrite(craftyo_move, craftyo_boardstate);
}

void ACHRead() {
  int r = 0;
  Somatic__Crafty *msg =
		  SOMATIC_GET_LAST_UNPACK(r, somatic__crafty,  &protobuf_c_system_allocator, BOARDSTATE_SIZE+MOVESTRING_SIZE, craftyo_chan);

  if(r != ACH_OK)
	  printf("ACH: Reading from channel returned %s\n", ach_result_to_string((ach_status_t)r));

  somatic__crafty__free_unpacked(msg, &protobuf_c_system_allocator);
  ACHTokenRead();
}

void ACHClose() {
  printf("ACH: Closing ACH\n");
  int r = ach_close(craftyo_chan);
  if(r != ACH_OK)
	  printf("ACH: %s\n", ach_result_to_string((ach_status_t)r));
//  free(craftyo_message->boardstate.data);
  free(craftyo_message->move);
  free(craftyo_message);
  ACHTokenClose();
}

#endif /* CHESSD_H_ */
