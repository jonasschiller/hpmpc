#pragma once
#include <cstdint>
#include <pthread.h>
#include <string>
#include <vector>
#include "../arch/DATATYPE.h"


typedef struct receiver_arguments {
  int player_count;
  int player_id;
  int connected_to;
  DATATYPE **received_elements;
  int inputs_size; //depricated
  std::string ip;
  int port;
  char *hostname;
  int rec_rounds;
  std::vector<int64_t> elements_to_rec;
  int total_rounds; //depricated
  //char *data;
  //char *length
} receiver_args;

typedef struct sender_arguments {
  DATATYPE **sent_elements;
  int inputs_size; //depricated
  int port;
  int player_id;
  int player_count;
  int connected_to;
  int send_rounds;
  std::vector<int64_t> elements_to_send;
  int total_rounds; //depricated
    #if PRE == 1
  uint64_t fetch_counter; // use to fetch sent in pre-processing round
    #endif

  //char *data;
} sender_args;

