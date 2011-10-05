/* -*- mode: C; c-basic-offset: 2  -*- */
#ifndef TOKEND_H_
#define TOKEND_H_

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

#define TOKEN_CHANNEL_DATA_SIZE sizeof(int)*(68+64)+sizeof(double)*64+100

static const char *token_chan_name = "crafty";
Somatic__Token* token_message;
ach_channel_t *token_chan;

// Initialize ACH Channel and all data
int ACHTokenInit() {
  // Channels
  int r = 0;
  printf("ACHT: Creating Channel\n");
  ach_create_attr_t attr;
  ach_create_attr_init(&attr);
  r = ach_create((char*)token_chan_name, 100, TOKEN_CHANNEL_DATA_SIZE, &attr);
  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));

  printf("ACHT: Opening Channel\n");
  token_chan = (ach_channel_t*) malloc (sizeof(ach_channel_t));
  if(token_chan == NULL) return 0;
  r = ach_open(token_chan, token_chan_name, NULL);

  if(r != ACH_OK) {
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  ach_chmod(token_chan, SOMATIC_CHANNEL_MODE );

  r = ach_flush(token_chan);

  if(r != ACH_OK) {
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
	  return 0;
  }

  // Data
  token_message = (Somatic__Token*)malloc(sizeof(Somatic__Token));
  if(token_message == NULL) return 0;
  somatic__token__init(token_message);

  token_message->type = malloc(100);
  token_message->iattr = malloc(sizeof(Somatic__Ivector));
  somatic__ivector__init(token_message->iattr);
  token_message->iattr->data = malloc(sizeof(int64_t)*(68+64));
  token_message->iattr->n_data = 68+64;

  token_message->fattr = malloc(sizeof(Somatic__Vector));
  somatic__vector__init(token_message->fattr);
  token_message->fattr->data = malloc(sizeof(double)*64);
  token_message->fattr->n_data = 64;

  printf("ACHT: Info\n"
		 "ACHT: Name: %s\n"
		 "ACHT: Size: %d\n", token_chan_name, somatic__token__get_packed_size(token_message));

  // Output data

  return 1;
}

void ACHTokenWrite(char* move, char* position) {
  printf("ACHT: Writing token %s\n", move);
  int n = 0;
  if((65 <= move[0] && move[0] <= 65+26)) n++;
  token_message->iattr->data[1] = move[n+0] - 96;
  token_message->iattr->data[0] = move[n+1] - 48;
  if(move[n+2] == 'x') n++;
  token_message->iattr->data[3] = move[n+2] - 96;
  token_message->iattr->data[2] = move[n+3] - 48;
  printf("Move %d,%d to %d,%d\n", (int)token_message->iattr->data[0], (int)token_message->iattr->data[1],
		  (int)token_message->iattr->data[2], (int)token_message->iattr->data[3]);
  int i = 0;
  for(i = 0; i < 128; i++) {
	  token_message->iattr->data[i+4] = position[i];
  }

  int r = SOMATIC_PACK_SEND(token_chan, somatic__token, token_message);

  if(r != ACH_OK)
	  printf("ACHT: Writing to channel returned %s\n", ach_result_to_string((ach_status_t)r));
}

void ACHTokenRead() {
  int r = 0;
  int i = 0, j = 0;
  printf("ACHT: Reading token\n");
  Somatic__Token *msg =
		  SOMATIC_GET_LAST_UNPACK(r, somatic__token,  &protobuf_c_system_allocator, TOKEN_CHANNEL_DATA_SIZE , token_chan);

  if(r == ACH_OK) {
	  printf("Move %d,%d to %d,%d\n", (int)msg->iattr->data[0], (int)msg->iattr->data[1],
			  (int)msg->iattr->data[2], (int)msg->iattr->data[3]);
	  for(i = 0; i < 16; i++) {
		  if(i == 8) printf("\n");
		  for(j = 0; j < 8; j++) {
			  printf("%4d ", (int)msg->iattr->data[4+(i*8)+j]);
		  }
		  printf("\n");
	  }
  } else
	  printf("ACHT: Reading from channel returned %s\n", ach_result_to_string((ach_status_t)r));

  somatic__token__free_unpacked(msg, &protobuf_c_system_allocator);
}

void ACHTokenClose() {
  printf("ACHT: Closing ACH\n");
  int r = ach_close(token_chan);
  if(r != ACH_OK)
	  printf("ACHT: %s\n", ach_result_to_string((ach_status_t)r));
  free(token_message);
}

#endif /* TOKEND_H_ */
