#pragma once
#include <stdbool.h> // bool
#include "../include/uthash.h"

// This header contains the datastructs used by the daemon:
// 1. enum command: the commands inserted into the json (a string) and used by the server to understand what it should do
// 
// 2. data_wrapper: the json is parsed into this struct
//
// 3. messages: a list of the messages the client still has to read
//
// 4. peer: the list of peers with messages that has to be read by a client

#define TOR_TIMEOUT 120000

enum command {
	SEND,
	RECV,
	UPDATE,
	GET_PEERS,
	HOST,
	END,
	ERR,
	EXIT,
};

struct data_wrapper {
	// contains the content of the message passed
	enum command cmd;
	char *id; // host
	int portno;
	char *msg;
	char *date;
};

struct message {
	char *content;
	char *date;
	enum command cmd;
	struct message  *next;
	struct message  *prev;
};

struct peer {
	// contains id of peers you are talking to
	// and a list of messages of each id
	char id[30];
	struct message *msg;
	UT_hash_handle hh; // hash table handler
};

struct message *
get_unread_message(const char *peerId);

bool
peer_exist (const char *id);

bool
insert_peer (const char *id);

struct peer *
get_list_head();

bool
insert_new_message  (const char *peerId, const char *content, enum command cmd);

char *
get_peer_list ();

void
clear_datastructs ();

void
free_data_wrapper (struct data_wrapper *data);
