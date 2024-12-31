#ifndef __HEADERS_H__
#define __HEADERS_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_SIZE 4096
#define NSIP  1
#define NSPORT 4011
#define MAX_CONNECTIONS 10


#define RED    "\033[0;31m"
#define GREEN  "\033[0;32m"
#define BLUE   "\033[0;34m"
#define YELLOW "\033[0;33m"
#define RESET  "\033[0m"
#define red(str)    RED    str RESET
#define green(str)  GREEN str RESET
#define blue(str)   BLUE   str RESET
#define yellow(str) YELLOW str RESET

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
    ERR_OPERATION_FAILURE_INFO = 509,

} ErrorCode;
ErrorCode res;
typedef struct connectst{
    int ns;
    int client;
}connectst;
#endif