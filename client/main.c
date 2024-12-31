#include"header.h"
#include <pthread.h>

// void* connect_and_wait(void* arg) {
//     int client_sock = connectns(ns_ip_address);
//     printf("Connected to naming server\n");
//     packet ack;
    
//     if (recv(client_sock, &ack, sizeof(ack), 0) == -1) {
//         perror("Send failed");
//         close(client_sock);
//     }
//     if(ack.type == SUCCESS_OK) {
//         printf("ACK packet received for WRITE\n");
//     } else {
//         printf("Error in connection\n");
//     }
//     close(client_sock);
//     return NULL;
// }

void communicate_with_ss(char *ipaddress, char *port, char *path, int type)
{   

    int cli_sock = connect_server(ipaddress, port);
    packet req;
    req.type = type;
    if (type != 3) {
        strcpy(req.data, path);
        printf("Sending packet to storage serverLDLD\n");
        if (send(cli_sock, &req, sizeof(req), 0)== -1) {
            perror("Send failed");
            close(cli_sock);
        }
    } else {
        char *token1 = strtok(path, "|");
        char *token2 = strtok(NULL, "|");
        printf("%s %s\n", token1, token2);
        if (token1 == NULL || token2 == NULL) {
            printf("Invalid path format\n");
            close(cli_sock);
            return;
        }
        strcpy(req.data, token1);
        if (send(cli_sock, &req, sizeof(req), 0) == -1) {
            perror("Send failed");
            close(cli_sock);
            return;
        }

        packet req_part;
        req_part.type = 30;
        size_t path_len = strlen(token2);
        size_t offset = 0;
        while (offset < path_len) {
            size_t chunk_size = 5;
            if (path_len - offset < chunk_size) {
                chunk_size = path_len - offset;
            }
            memset(req_part.data, 0, sizeof(req_part.data));
            strncpy(req_part.data, token2 + offset, chunk_size);
            printf("Sending chunk: %s\n", req_part.data);
            req_part.data[chunk_size] = '\0';
            if (send(cli_sock, &req_part, sizeof(req_part), 0) == -1) {
                perror("Send failed");
                close(cli_sock);
                return;
            }
            offset += chunk_size;
        }
        req_part.type = 25; // Stop packet type
        strcpy(req_part.data, "stop");
        if (send(cli_sock, &req_part, sizeof(req_part), 0) == -1) {
            perror("Send failed");
            close(cli_sock);
            return;
        }
    }

    // Reader Packet sent
    packet reply;
    memset(&reply, 0, sizeof(packet));
    if (type == 1) {
        while (1) {
            int status = recv(cli_sock, &reply, sizeof(reply), 0);
            if (status == -1) {
                perror("Receiving data failed");
                break;
            }
            if (reply.type == 100) {
                printf("%s", reply.data);
            } else if (reply.type == STOP) {
                break;
            } else {
                printf("Error in read %d\n", reply.type);
                break;
            }
            printf("HERE\n");
        }
    } else {
        while (1) {
            int status = recv(cli_sock, &reply, sizeof(reply), 0);
            if (status == -1) {
                perror("Receiving data failed");
                break;
            }
            if (reply.type == SUCCESS_OK) {
                printf("%s\n", reply.data);
            } else if (reply.type == STOP) {
                break;
            } else {
                printf("Error in write %d\n", reply.type);
                break;
            }
        }
        
    }
    close(cli_sock);
    return ;

}

int main(){
     printf(green("WELCOME TO NETWORK FILE SYSTEM - CLIENT !!\n"));
     char cmd[MAX_SIZE];

    char ns_ip_address[64];
    printf("Enter the IP address of the Naming Server: ");
    if (fgets(ns_ip_address, sizeof(ns_ip_address), stdin) == NULL) {
        perror("Failed to read IP address");
        //close(server_to_NS);
        exit(EXIT_FAILURE);
    }
    ns_ip_address[strcspn(ns_ip_address, "\n")] = '\0'; // Remove newline character if present

    while(1){
        printf("-------Enter command to perform-------\n");
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = '\0';
        char *token = strtok(cmd, " ");
        int cont_flag=0;
        printf("tbty");
        if (strcmp(token, "READ") == 0) {
            token=strtok(NULL, " ");
                if (token == NULL) {
                    printf(red("Invalid command\n"));
                    continue;
                }
                char file_path[1024];
                strcpy(file_path,token);
                int client_sock  = connectns(ns_ip_address);
                printf("Connected to naming server\n");
                packet request;
                request.type = READ;
                strcpy(request.data, token);
                if (send(client_sock, &request, sizeof(request), 0)== -1)
                {
                    perror("Send failed");
                    close(client_sock);
                }
                printf("packet sent\n");
                packet reply;
                int status = recv(client_sock, &reply, sizeof(reply), 0);
               // printf("%d %s\n",reply.type,reply.data);
                if(status == -1)
                {
                    perror("Receiving data failed");
                    
                }
                else if(reply.type ==SUCCESS_OK){
                printf("%d %s\n",reply.type,reply.data);
                }
                else {
                    printf("Error %d\n", reply.type);
                    cont_flag=1;
                    close(client_sock);
                    // continue;
                }
                if(cont_flag==1)
                {
                    continue;
                }
                char *port;
                char *ipaddress;
                ipaddress = strtok(reply.data, "|");
                port = strtok(NULL, "|");
                printf("HERE %s %s\n", ipaddress ,port);
                close(client_sock);
                communicate_with_ss(ipaddress, port, file_path, READ);
                
        }
        else if(strcmp(token, "WRITE") == 0){
            token=strtok(NULL, " ");
            if (token == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            char dataToSend[1024];
            strcpy(dataToSend,token);
            char *token2 = strtok(NULL, "");
            if (token2 != NULL) {
                strcat(dataToSend, "|");
                strcat(dataToSend, token2);
            }
            int client_sock  = connectns(ns_ip_address);
            printf("Connected to naming server\n");
            packet request;
            request.type = WRITE;
            strcpy(request.data, token);
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            printf("packet sent\n");
            packet reply;
            int status = recv(client_sock, &reply, sizeof(reply), 0);
            printf("%d %s\n",reply.type,reply.data);
            if(status == -1)
            {
                perror("Receiving data failed");
                
            }
            else if(reply.type ==SUCCESS_OK){
            printf("%d %s\n",reply.type,reply.data);
            }
            else {
                printf("Error %d\n", reply.type);
                cont_flag=1;
                close(client_sock);
                // continue;
            }
            if(cont_flag==1)
            {
                continue;
            }
            char *port;
            char *ipaddress;
            ipaddress = strtok(reply.data, "|");
            port = strtok(NULL, "|");
            close(client_sock);
            communicate_with_ss(ipaddress, port, dataToSend, WRITE);
        } else if (strcmp(token, "ASYNCWRITE") == 0) {
            token=strtok(NULL, " ");
            if (token == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            char dataToSend[1024];
            strcpy(dataToSend,token);
            char *token2 = strtok(NULL, "");
            if (token2 != NULL) {
                strcat(dataToSend, "|");
                strcat(dataToSend, token2);
            }
            int client_sock  = connectns(ns_ip_address);
            printf("Connected to naming server\n");
            packet request;
            request.type = ASYNCWRITE;
            strcpy(request.data, token);
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            printf("packet sent\n");
            packet reply;
            int status = recv(client_sock, &reply, sizeof(reply), 0);
            printf("%d %s\n",reply.type,reply.data);
            if(status == -1)
            {
                perror("Receiving data failed");
                
            }
            else if(reply.type ==SUCCESS_OK){
            printf("%d %s\n",reply.type,reply.data);
            }
            else {
                printf("Error %d\n", reply.type);
                cont_flag=1;
                close(client_sock);
            }
            if(cont_flag==1)
            {
                continue;
            }
            char *port;
            char *ipaddress;
            ipaddress = strtok(reply.data, "|");
            port = strtok(NULL, "|");
            close(client_sock);
            communicate_with_ss(ipaddress, port, dataToSend, ASYNCWRITE);

        } else if(strcmp(token, "LIST") == 0){
            int client_sock  = connectns(ns_ip_address);
            printf("Connected to naming server\n");
            packet request;
            request.type = LIST;
            
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            printf("packet sent\n");
            packet reply;
            int status;
            while (1) {
                status = recv(client_sock, &reply, sizeof(reply), 0);
                if (status == -1) {
                    perror("Receiving data failed");
                    break;
                }
                if (reply.type == STOP) {
                    break;
                }
                printf("%s\n", reply.data);
            }
            printf("HERsdfsdE\n");
            close(client_sock);
        }
        else if(strcmp(token, "INFO") == 0){
            token=strtok(NULL, " ");
                if (token == NULL) {
                    printf(red("Invalid command\n"));
                    continue;
                }
                char file_path[1024];
                strcpy(file_path,token);
                int client_sock  = connectns(ns_ip_address);
                printf("Connected to naming server\n");
                packet request;
                request.type = INFO;
                strcpy(request.data, token);
                if (send(client_sock, &request, sizeof(request), 0)== -1)
                {
                    perror("Send failed");
                    close(client_sock);
                }
                printf("packet sent\n");
                packet reply;
                int status = recv(client_sock, &reply, sizeof(reply), 0);
                if (status == -1)
                {
                    perror("Receiving data failed");
                    close(client_sock);
                    continue;
                }
                if(reply.type == ERR_NOT_FOUND_FILE)
                {
                    printf("File not found\n");
                }
                if(reply.type == SUCCESS_OK)
                {
                    printf("%d %s\n", reply.type, reply.data);
                }
                else{
                    printf("Error %d\n", reply.type);
                }
                //printf("%d %s\n", reply.type, reply.data);
                close(client_sock);
        }
        
        else if(strcmp(token, "CREATE") == 0){
            token=strtok(NULL, " ");
            if (token == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            char dataToSend[1024];
            strcpy(dataToSend,token);
            char *token2 = strtok(NULL, "");
            if (token2 == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            if (token2 != NULL) {
                strcat(dataToSend, "|");
                strcat(dataToSend, token2);
            }
            int client_sock  = connectns(ns_ip_address);
            printf("Connected to naming server\n");
            packet request;
            request.type = CREATE;
            strcpy(request.data, dataToSend);
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            printf("packet sent\n");
            packet reply;
            int status = recv(client_sock, &reply, sizeof(reply), 0);
            if(status == -1)
            {
                perror("Receiving data failed");
            }
            if (reply.type==100)
            {
                printf("Creation of Directory or File succesfull \n");
            }
            else
            {   
                printf("Creation of Directory or File not succesfull ERROR : %d \n",reply.type); // Error Not succesfull

            }
            close(client_sock);
        }
        else if(strcmp(token, "CREATEFOLDER") == 0){
            token=strtok(NULL, " ");
            if (token == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            char dataToSend[1024];
            strcpy(dataToSend,token);
            char *token2 = strtok(NULL, "");
            if (token2 == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            if (token2 != NULL) {
                strcat(dataToSend, "|");
                strcat(dataToSend, token2);
            }
            int client_sock  = connectns(ns_ip_address);
            printf("Connected to naming server\n");
            packet request;
            request.type = CREATEFOLDER;
            strcpy(request.data, dataToSend);
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            printf("packet sent\n");
            packet reply;
            int status = recv(client_sock, &reply, sizeof(reply), 0);
            if(status == -1)
            {
                perror("Receiving data failed");
            }
            if (reply.type==100)
            {
                printf("Creation of Directory or File succesfull \n");
            }
            else
            {   
                printf("Creation of Directory or File not succesfull ERROR : %d \n",reply.type); // Error Not succesfull
            }
            close(client_sock);
        }
        else if(strcmp(token, "DELETE") == 0){
            token=strtok(NULL, " ");
            if (token == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            char path[MAX_SIZE];
            strcpy(path,token);
            int client_sock  = connectns(ns_ip_address);
            printf("Connected to naming server\n");
            packet request;
            request.type = DELETE;
            strcpy(request.data, path);
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            printf("packet sent\n");
            packet reply;
            int status = recv(client_sock, &reply, sizeof(reply), 0);
            if(status == -1)
            {
                perror("Receiving data failed");
            }
            if (reply.type==100)
            {
                printf("DELETION of Directory or File succesfull \n");
            }
            else if(reply.type==301)
            {   
                printf("File not found ERROR : %d \n",reply.type); // Error Not succesfull
            }
            else
            {   
                printf("Deletion of Directory or File not succesfull ERROR : %d \n",reply.type); // Error Not succesfull
            }

            close(client_sock);
        }
        
        else if(strcmp(token, "COPY") == 0){
            token=strtok(NULL, " ");
            if (token == NULL) {
                printf(red("Invalid command\n"));
                continue;
            }
            char dataToSend[1024];
            strcpy(dataToSend,token);
            char *token2 = strtok(NULL, "");
            if (token2 != NULL) {
                strcat(dataToSend, "|");
                strcat(dataToSend, token2);
            }
            int client_sock  = connectns(ns_ip_address);
            
            packet request;
            request.type = COPY;
            strcpy(request.data, dataToSend);
            if (send(client_sock, &request, sizeof(request), 0)== -1)
            {
                perror("Send failed");
                close(client_sock);
            }
            packet reply;
            int status = recv(client_sock, &reply, sizeof(reply), 0);
            if(status == -1)
            {
                perror("Receiving data failed");
            }
            if (reply.type==100)
            {
                printf("COPY of file succesfull \n");
            }
            else if(reply.type==301)
            {   
                printf("File not found ERROR : %d \n",reply.type); // Error Not succesfull
            }
                //functiondelete(path);
            else
            {   
                printf("COPY not succesfull ERROR : %d \n",reply.type); // Error Not succesfull
            }
            close(client_sock);
        }
        else if(strcmp(token, "EXIT") == 0){
            return 0;
        }
        else{
            printf(red("Invalid command\n"));
            continue;
            }
    }
    return 0;
}