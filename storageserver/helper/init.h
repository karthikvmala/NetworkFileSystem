#ifndef INIT_H
#define INIT_H

#include "header.h"
int findport();

int findport() {
    int port = 0;
    int server_socket;
    struct sockaddr_in server_addr;

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(0);

    // Bind the socket to the specified IP and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Get the port assigned by the OS
    socklen_t len = sizeof(server_addr);
    if (getsockname(server_socket, (struct sockaddr *)&server_addr, &len) == -1)
    {
        perror("Getsockname failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    port = ntohs(server_addr.sin_port);

    // Clean up
    close(server_socket);

    return port;
}

#endif