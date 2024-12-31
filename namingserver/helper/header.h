#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tries.h"
#include "cache.h"
#include "log.h"

#define MAX_DATA_LENGTH 1024
#define MAX_PATHS 10
#define IP_LEN 64
#define MAX_SS 10        // Maximum number of Storage Servers


#define NM_CLIENT_PORT 4012
#define NM_SS_PORT_LISTEN 4011
#define MAX_CONNECTIONS 10
#define MAX_SIZE 4096
#define STOP 20

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

typedef struct ip_and_socket {
    char ip[IP_LEN];
    int socket;
} ip_and_socket;

typedef struct Backup {
    int status;
    int backups[2];
    int is_backup0;
    int is_backup1;
} Backup;

extern int curr_cache_write_index;

// tries.h
struct trie_node *create_trie_node();
int insert_path(struct trie_node *root, char *String_to_search, int ssid);
int search_path(struct trie_node *root, char *key);
void print_paths(struct trie_node *root);
int delete_path(struct trie_node *root, char *string_to_delete);

// chache.h
void init_cache();
void init_cache(LRUCache *cache);
void update_lru(LRUCache *cache, int index);
void use_cache(LRUCache *cache, int index);
IpPortPair get(LRUCache *cache, const char *key);
void put(LRUCache *cache, const char *key, const char *ip, int port, int port_nm, int idx);
void delete_from_cache(LRUCache *cache, const char *key);
void print_cache(LRUCache *cache);

// log.h
void printLog(int signum);
int Log(int ss_or_client, int ss_id, int port, int req_type, char *req_data);

////////////////////////ERROR CODES/////////////////////////
typedef enum {
    // Success
    SUCCESS_OK = 100,
    ERR_NOT_FOUND_FILE = 301,
    ERR_NOT_FOUND_SERVER = 302,
    ERR_NOT_FOUND_RESOURCE = 303,
    ERR_INVALID_OPERATION = 401,
    ERR_OPERATION_FAILURE_REQUEST = 501,
    ERR_OPERATION_FAILURE_CREATE = 502,
    ERR_OPERATION_FAILURE_DELETE = 503,
    ERR_OPERATION_FAILURE_COPY = 504,
    ERR_OPERATION_FAILURE_PASTE = 505,
    ERR_OPERATION_FAILURE_APPEND = 506,
    ERR_OPERATION_FAILURE_READ = 507,
    ERR_OPERATION_FAILURE_WRITE = 508,
    ERR_OPERATION_FAILURE_INFO = 509

} ErrorCode;
ErrorCode res;