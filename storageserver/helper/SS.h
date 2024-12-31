#ifndef SS_H
#define SS_H

#include "header.h"
#include "init.h"
#include "tries.h"
#define MAX_PATHS 3
#define IP_LEN 64

// init.h
void init_server();
void register_with_ns(int client_socket);

typedef struct {
    char ip[IP_LEN];
    int port_nm;
    int port_client;
    char accessible_paths[4096];
    int num_paths;
    trie_node* root;
} StorageServerInfo;

typedef struct Packet {
    int  type;              // Request type would determine whether it is an acknowledgement, query, response, data etc.
    char data[MAX_SIZE];     // All the communication happens in the form of strings
} packet;

#endif