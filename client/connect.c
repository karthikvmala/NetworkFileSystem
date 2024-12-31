#include "header.h"
int connectns(char* ns_ip_address)
{
    int client_sock;
    struct sockaddr_in serv_addr;

    // creating a socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // defining server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(NSPORT);
    
    if (inet_pton(AF_INET, ns_ip_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    printf("%d %d %s\n", client_sock, serv_addr.sin_port, ns_ip_address);
    while(1){
        if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  
        {
            perror(red("Connecftion failed"));
            sleep(1);
        }
        else
        {   
            printf(red("Connected to server"));
            break;
        }
    }
    return client_sock;
}

int connect_server(char *IP, char *PORT)
{
    int client_sock;
    struct sockaddr_in serv_addr;

    // creating a socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // defining server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(PORT));
    
    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }
    while(1){
    if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  
    {
        perror(red("Connection failed"));
        return 0;
    }
    else
    {   
        printf(red("Connected to server"));
        break;
    }
    }
    return client_sock;
}