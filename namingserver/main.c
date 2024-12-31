#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "helper/header.h"

LRUCache cache;
int connect_server(char *IP, int PORT)
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
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET,IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supportedasdfasdfasdf");
        exit(EXIT_FAILURE);
    }
    while(1){
    if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)  
    {
        perror("Connection failed");
        return 0;
    }
    else
    {   
        printf("Connected to server");
        break;
    }
    }
    return client_sock;
}

StorageServerInfo ss_list[MAX_SS];
Backup b_list[MAX_SS];
int ss_count = 0;     // Count of registered Storage Servers
pthread_mutex_t lock; // Mutex lock for thread safety

int counter = 0;
ErrorCode func(int index_src,int back1_index){
    char paths2[256][MAX_DATA_LENGTH];
    int cnt = 0;
    add_paths_to_array(paths2, &cnt, ss_list[index_src].root);
    //print_paths(ss_list[index_src].root);
    printf("Paths: %d\n", cnt);
    for (int i = 0; i < cnt; i++) {
        printf("%s\n", paths2[i]);
    }
    char paths[256][MAX_DATA_LENGTH];
    for (int i = 0; i < cnt; i++) {
        strcpy(paths[i], paths2[i]);
    }
    packet response_packet;
    StorageServerInfo temp_src = ss_list[index_src];
    StorageServerInfo temp_dest = ss_list[back1_index];
    
    char dest[1024];
    sprintf(dest, "./backup%d", index_src);
    
    int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
        memset(response_packet.data, 0, sizeof(response_packet.data));
        strcat(response_packet.data, dest);
        //char *path = paths[i];
        // if (path[0] == '.') {
        //     path++; // Skip the initial dot
        // }
        //strcat(response_packet.data, path);
        response_packet.type=13;
        if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
            perror("Send failed");
            return ERR_OPERATION_FAILURE_CREATE;
            close(dest_socket);
        }
        memset(response_packet.data, 0, sizeof(response_packet.data));
        if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
            perror("Receive failed");
            return ERR_OPERATION_FAILURE_CREATE;
            close(dest_socket);
            
        }
    for (int i = 0; i < cnt; i++) {
    if (strstr(paths[i], ".txt") != NULL || strstr(paths[i], ".c") != NULL || strstr(paths[i], ".mp3") != NULL||strstr(paths[i], ".h") != NULL) {
        int ss_socket = connect_server(temp_src.ip, temp_src.port_nm);
    
        response_packet.type=10;
        strcpy(response_packet.data,paths[i]);
        // printf("%s %d\n",response_packet.data,response_packet.type);
        if (send(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
            perror("Send failed");
            close(ss_socket);
            
        }

        if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
            perror("Receive failed");
            return ERR_OPERATION_FAILURE_COPY;
            close(ss_socket);
        }
        close(ss_socket);
        if(response_packet.type==100){
            int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
            response_packet.type=11;
        strcat(response_packet.data, "|");
        strcat(response_packet.data, dest);
        char *path = paths[i];
        if (path[0] == '.') {
            path++; // Skip the initial dot
        }
        strcat(response_packet.data, path);
        // printf("Response packet type: %s\n", response_packet.data);
            if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                perror("Send failed");
                return ERR_OPERATION_FAILURE_PASTE;
                close(dest_socket);
            }
            
            if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                perror("Receive failed");
                return ERR_OPERATION_FAILURE_PASTE;
                close(dest_socket);
            }
            if(response_packet.type==100){
                // res2=SUCCESS_OK;
            }
            else{
                return ERR_OPERATION_FAILURE_PASTE;
            }
            close(dest_socket);
        }
        else{
            return ERR_OPERATION_FAILURE_PASTE;
        }
    }
    else{
        //create folder
        int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
        memset(response_packet.data, 0, sizeof(response_packet.data));
        strcat(response_packet.data, dest);
        char *path = paths[i];
        if (path[0] == '.') {
            path++; // Skip the initial dot
        }
        strcat(response_packet.data, path);
        response_packet.type=13;
        if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
            perror("Send failed");
            return ERR_OPERATION_FAILURE_CREATE;
            close(dest_socket);
        }
        memset(response_packet.data, 0, sizeof(response_packet.data));
        if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
            perror("Receive failed");
            return ERR_OPERATION_FAILURE_CREATE;
            close(dest_socket);
            
        }
        
            if(response_packet.type==100){
            //done successfully
        }
    else{
        close(dest_socket);
        return ERR_OPERATION_FAILURE_PASTE;
        }
        close(dest_socket);
        }
    }

    return SUCCESS_OK;
}


int backup_search(int x, int y)
{
    int temp = counter;
    printf("HERE\n");
    do
    {
        if (b_list[counter].status == 1 && counter != x && counter != y)
        {
            return counter;
        }
        counter = (counter + 1) % ss_count;
    } while (counter != temp);
    return -1;
}

// void register_storage_server(char *ip, int client_socket)
// {
//     StorageServerInfo ss_info;
//     int read_size;

//     // Receive the Storage Server information from the client
//     if ((read_size = recv(client_socket, &ss_info, sizeof(ss_info), 0)) > 0)
//     {
//         pthread_mutex_lock(&lock);
//         strcpy(ss_info.ip, ip);
//         int backed_up = 0;
//         ss_list[ss_count] = ss_info;

//         for(int i = 0; i < ss_count; i++)
//         {
//             if(ss_list[i].port_nm == ss_info.port_nm)
//             {
//                 b_list[i].status = 1;
//                 backed_up = 1;
//                 printf("Storage Server already registered");
//                 break;;
//             }
//         }
//         if(!backed_up){
//             b_list[ss_count].status = 1;
//             b_list[ss_count].backups[0] = -1;
//             b_list[ss_count].backups[1] = -1;
//             b_list[ss_count].is_backup0 = -1;
//             b_list[ss_count].is_backup1 = -1;
//             ss_count++;
//         }

//         printf("Registered Storage Server:\n");
//         printf("IP: %s\n", ss_info.ip);
//         printf("NM Port: %d\n", ss_info.port_nm);
//         printf("Client Port: %d\n", ss_info.port_client);
//         printf("Paths: %d\n", ss_info.num_paths);

//         // add ssid in the struct and modify it so that it inserts the ssid in the trie
//         // Initialize a new trie node for this storage server
//         ss_list[ss_count - 1].root = create_trie_node(); 

//         // Tokenize the accessible paths from ss_info and insert each path individually
//         char *token = strtok(ss_info.accessible_paths, "|");
//         int path_index = 0;

//         while (token != NULL) {
//             printf("Inserting token %d: %s\n", path_index, token);
            
            
//             token[strcspn(token, "\n")] = '\0'; 
//             insert_path(ss_list[ss_count - 1].root, token, ss_count - 1);
            
//             // Move to the next token
//             token = strtok(NULL, "|");
//             path_index++;
//         }

//         // Print paths of the newly added storage server for verification
//         printf("Paths in newly registered server (ss_count - 1):\n");
//         print_paths(ss_list[ss_count - 1].root);

//         printf("\n=====\n");

//         // Print paths of the first server to verify it remains unchanged
//         printf("Paths in first registered server:\n");
//         print_paths(ss_list[0].root);
//         printf("\n=====\n");


//         pthread_mutex_unlock(&lock);
//     }
//     else
//     {
//         perror("Failed to receive Storage Server information");
//     }
// }

void register_storage_server(char *ip, int client_socket)
{
    StorageServerInfo ss_info;
    int read_size;

    // Receive the Storage Server information from the client
    if ((read_size = recv(client_socket, &ss_info, sizeof(ss_info), 0)) > 0)
    {
        pthread_mutex_lock(&lock);
        strcpy(ss_info.ip, ip);
        int backed_up = 0;
        for (int i = 0; i < ss_count; i++)
        {
            if (ss_list[i].port_nm == ss_info.port_nm)
            {
                // b_list[i].status = 1;
                // backed_up = 1;
                // printf("Storage Server already registered");
                // break;
                // yeyyyyyyyyyyyyyyy
                strcpy(ss_list[i].ip, ss_info.ip);
                b_list[i].status = 1;
                backed_up = 1;
                printf("Storage Server already registered");
                char paths2[256][MAX_DATA_LENGTH];
                int cnt = 0;
                add_paths_to_array(paths2, &cnt, ss_list[i].root);
                for (int j = 0; j < cnt; j++)
                {
                    delete_path(ss_list[i].root, paths2[j]);
                }

                char temp_path[sizeof(ss_info.accessible_paths)];
                strcpy(temp_path, ss_info.accessible_paths);
                char *token = strtok(temp_path, "|");
                while (token != NULL)
                {
                    // wrong:
                    insert_path(ss_list[i].root, token, i);
                    token = strtok(NULL, "|");
                }
            }
        }
        if (!backed_up)
        {
            ss_list[ss_count] = ss_info;
            b_list[ss_count].status = 1;
            b_list[ss_count].backups[0] = -1;
            b_list[ss_count].backups[1] = -1;
            b_list[ss_count].is_backup0 = -1;
            b_list[ss_count].is_backup1 = -1;
            ss_count++;
        }

        printf("Registered Storage Server:\n");
        printf("IP: %s\n", ss_info.ip);
        printf("NM Port: %d\n", ss_info.port_nm);
        printf("Client Port: %d\n", ss_info.port_client);
        printf("Paths: %d\n", ss_info.num_paths);

        // add ssid in the struct and modify it so that it inserts the ssid in the trie
        // Initialize a new trie node for this storage server
        if (backed_up == 0)
        {
            ss_list[ss_count - 1].root = create_trie_node();

            // Tokenize the accessible paths from ss_info and insert each path individually
            char *token = strtok(ss_info.accessible_paths, "|");
            int path_index = 0;

            while (token != NULL)
            {
                printf("Inserting token %d: %s\n", path_index, token);

                token[strcspn(token, "\n")] = '\0';
                insert_path(ss_list[ss_count - 1].root, token, ss_count - 1);

                // Move to the next token
                token = strtok(NULL, "|");
                path_index++;
            }
            // Print paths of the newly added storage server for verification
            printf("Paths in newly registered server (ss_count - 1):\n");
            print_paths(ss_list[ss_count - 1].root);

            printf("\n=====\n");
        }

        // Print paths of the first server to verify it remains unchanged
        printf("Paths in first registered server:\n");
        print_paths(ss_list[0].root);
        printf("\n=====\n %d", ss_count);

        pthread_mutex_unlock(&lock);
    }
    else
    {
        perror("Failed to receive Storage Server information");
    }
}

void *handle_ss_registration(void *socket_desc)
{
    ip_and_socket client_socket = *(ip_and_socket *)socket_desc;
    register_storage_server(client_socket.ip, client_socket.socket);
    close(client_socket.socket);
    return NULL;
}

pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;

void *handle_client_operations(void *socket_desc) {
    int client_socket = *(int *)socket_desc;
    packet Packet;
    int read_size, index = -1;

    while ((read_size = recv(client_socket, &Packet, sizeof(Packet), 0)) > 0) {
        // pthread_mutex_lock(&lock3);
        Log(0, -1, client_socket, Packet.type, Packet.data);
        if (Packet.type == 1 || Packet.type == 2 || Packet.type == 3) {
            printf("Received READ request for file: %s\n", Packet.data);
            pthread_mutex_lock(&lock);
            IpPortPair temp = get(&cache, Packet.data);
            int file_found = 0;

            if (temp.port == -1) {
                for (int i = 0; i < ss_count; i++) {
                    if (search_path(ss_list[i].root, Packet.data) != -1) {
                        file_found = 1;
                        index = i;
                        break;
                    }
                }
                if(file_found==1){
                put(&cache, Packet.data, ss_list[index].ip, ss_list[index].port_client, ss_list[index].port_nm, index);
                temp = get(&cache, Packet.data);
                }
            } 
            else{
                file_found=1;
            }
            print_cache(&cache);
            packet response_packet;
            ErrorCode res2;
            if (file_found) {
                res2=SUCCESS_OK;
                snprintf(response_packet.data, sizeof(response_packet.data), "%s|%d", temp.ip, temp.port);
            } else {
                res2 = ERR_NOT_FOUND_FILE;
                strcpy(response_packet.data, "File not found");
            }
            response_packet.type = res2;
            if (send(client_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                perror("Send failed");
            }
            pthread_mutex_unlock(&lock);
        }
        if (Packet.type == 4 || Packet.type == 5) {
            printf("Received request for file/folder: %s\n", Packet.data);
            pthread_mutex_lock(&lock);
            char temp_data[sizeof(Packet.data)];
            strcpy(temp_data, Packet.data);
            char *token = strtok(temp_data, "|");
            char path[256];
            strcpy(path, token);
            token = strtok(NULL, "|");
            char file_to_add[256];
            strcpy(file_to_add, token);
            int file_found = 0;
            IpPortPair temp = get(&cache, path);
            if (temp.port_nm == -1) {
                for (int i = 0; i < ss_count; i++) {
                    if (search_path(ss_list[i].root, path) != -1) {
                        file_found = 1;
                        index = i;
                        break;
                    }
                }
                if(file_found==1){
                    put(&cache, path, ss_list[index].ip, ss_list[index].port_client, ss_list[index].port_nm, index);
                    temp = get(&cache, path);
                }
            } 
            else{
                file_found=1;
            }
            print_cache(&cache);
            ErrorCode res2;

           
            packet response_packet;
            if(file_found){
                printf("%s %d\n", temp.ip, temp.port_nm);
                int ss_socket = connect_server(temp.ip, temp.port_nm);
                
                if (send(ss_socket, &Packet, sizeof(Packet), 0) < 0) {
                    perror("Send failed");
                    close(ss_socket);
                    continue;
                }
                
                if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                    perror("Receive failed");
                    close(ss_socket);
                    continue;
                }
                res2 = response_packet.type;
                close(ss_socket);
            }
            else{
                res2 = ERR_NOT_FOUND_FILE;
            }
          
            // TODO: put the new path in trie and add it to trie start
            if (res2 == 100) {
                printf("insdie res2==100\n");
                char new_path[4096];
                snprintf(new_path, sizeof(new_path), "%s/%s", path, file_to_add);
                printf("New path: %s\n", new_path);
                insert_path(ss_list[temp.idx].root, new_path,temp.idx);
                put(&cache, new_path, ss_list[temp.idx].ip, ss_list[temp.idx].port_client, ss_list[temp.idx].port_nm, temp.idx);
                //print_paths(ss_list[temp.idx].root);
            }
            // end

            // After creating the file/folder, you can send a response packet back to the client
            printf("statement3\n");
            packet response_packet_cl;
            response_packet_cl.type = res2;
            strcpy(response_packet_cl.data, response_packet.data);
            printf("Response packet type: %s\n", response_packet_cl.data);
            printf("statement4\n");
            if (send(client_socket, &response_packet_cl, sizeof(response_packet_cl), 0) < 0) {
                perror("Send failed");
            }
            printf("statement5\n");
            Log(1, index, client_socket, response_packet_cl.type, response_packet_cl.data);
            printf("statement6\n");
            pthread_mutex_unlock(&lock);
        } else if (Packet.type == 7) {
            printf("Received other request for file/folder: %s\n", Packet.data);
            pthread_mutex_lock(&lock);
            char temp_data[sizeof(Packet.data)];
            strcpy(temp_data, Packet.data);
            char *token = strtok(temp_data, "|");
            char path[256];
            strcpy(path, token);
            int file_found = 0;
            IpPortPair temp = get(&cache, path);
            if (temp.port_nm == -1) {
                for (int i = 0; i < ss_count; i++) {
                    if (search_path(ss_list[i].root, path) != -1) {
                        file_found = 1;
                        index = i;
                        break;
                    }
                }
                if(file_found==1){
                    put(&cache, path, ss_list[index].ip, ss_list[index].port_client, ss_list[index].port_nm, index);
                    temp = get(&cache, path);
                }
            } 
            else{
                file_found=1;
            }
            print_cache(&cache);
            ErrorCode res2;

           
            packet response_packet;
            if(file_found){
                printf("%s %d\n", temp.ip, temp.port_nm);
                int ss_socket = connect_server(temp.ip, temp.port_nm);
                
                if (send(ss_socket, &Packet, sizeof(Packet), 0) < 0) {
                    perror("Send failed");
                    close(ss_socket);
                    continue;
                }
                
                if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                    perror("Receive failed");
                    close(ss_socket);
                    continue;
                }
                res2 = response_packet.type;
                close(ss_socket);
            }
            else{
                res2 = ERR_NOT_FOUND_FILE;
            }

            // After creating the file/folder, you can send a response packet back to the client
            printf("statement3\n");
            packet response_packet_cl;
            response_packet_cl.type = res2;
            strcpy(response_packet_cl.data, response_packet.data);
            printf("Response packet type: %s\n", response_packet_cl.data);
            printf("statement4\n");
            if (send(client_socket, &response_packet_cl, sizeof(response_packet_cl), 0) < 0) {
                perror("Send failed");
            }
            printf("statement5\n");
            Log(1, index, client_socket, response_packet_cl.type, response_packet_cl.data);
            printf("statement6\n");
            pthread_mutex_unlock(&lock);
        } if(Packet.type==6){
            printf("Received  Delete request for file/folder: %s\n", Packet.data);
            pthread_mutex_lock(&lock);
            
            
            int file_found = 0;
            IpPortPair temp = get(&cache, Packet.data);
            if (temp.port_nm == -1) {
                for (int i = 0; i < ss_count; i++) {
                    if (search_path(ss_list[i].root, Packet.data) != -1) {
                        file_found = 1;
                        index = i;
                        break;
                    }
                }
                if(file_found==1){
                put(&cache, Packet.data, ss_list[index].ip, ss_list[index].port_client, ss_list[index].port_nm, index);
                temp = get(&cache, Packet.data);
                }
            } 
            else{
                file_found=1;
            }
            print_cache(&cache);
            ErrorCode res2;

            packet response_packet;
            if(file_found){
                printf("statement1\n");
                printf("%s %d\n", temp.ip, temp.port_nm);
                int ss_socket = connect_server(temp.ip, temp.port_nm);
                printf("statement2\n");
                if (send(ss_socket, &Packet, sizeof(Packet), 0) < 0) {
                    perror("Send failed");
                    close(ss_socket);
                    continue;
                }
                
                if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                    perror("Receive failed");
                    close(ss_socket);
                    continue;
                }
                res2 = response_packet.type;
                close(ss_socket);
            }
            else{
                res2 = ERR_NOT_FOUND_FILE;
            }

            if (res2 == 100) {
                delete_path(ss_list[temp.idx].root, Packet.data);
                delete_from_cache(&cache, Packet.data);
            }

            // After deleting the file/folder, you can send a response packet back to the client
            packet response_packet_cl;
            response_packet_cl.type = res2;
            printf("Response packet type: %d\n", response_packet_cl.type);
            if (send(client_socket, &response_packet_cl, sizeof(response_packet_cl), 0) < 0) {
                perror("Send failed");
            }
            Log(1, index, client_socket, response_packet_cl.type, response_packet_cl.data);
            
            pthread_mutex_unlock(&lock);
        }
        if(Packet.type==8){
            // List all accessible paths
            
            // char total_buffer[BUFFER_SIZE];
            
            // size_t buffer_len = 0;
            // for (int i = 0; i < ss_count; i++) {
            //     char buffer[BUFFER_SIZE];
            //     memset(buffer, 0, sizeof(buffer));
            //     collect_paths(ss_list[i].root, buffer, &buffer_len);
            //     strcat(total_buffer, buffer);
            //     buffer_len = 0;
            // }
           // printf("Accessible paths:\n%s\n", total_buffer);
           char total_buffer[BUFFER_SIZE];
            size_t buffer_len = 0;  // Tracks the current length of total_buffer

            memset(total_buffer, 0, sizeof(total_buffer));  // Clear total_buffer at the beginning
            printf("SS count: %d\n", ss_count);

            for (int i = 0; i < ss_count; i++) {
                // print_paths(ss_list[i].root);
                char paths2[256][MAX_DATA_LENGTH];
                int cnt = 0;
                
                add_paths_to_array(paths2, &cnt, ss_list[i].root);  // Populate paths2 array and cnt
                
                for (int j = 0; j < cnt; j++) {
                    size_t path_len = strlen(paths2[j]);

                    // Check if adding the next path would exceed total_buffer's size
                    if (buffer_len + path_len + 1 < BUFFER_SIZE) {
                        strcat(total_buffer, paths2[j]);
                        strcat(total_buffer, "\n");  // Optional: add newline to separate paths
                        buffer_len += path_len + 1;  // Update buffer_len to include new path and newline
                    } else {
                        fprintf(stderr, "Warning: total_buffer size exceeded!\n");
                        break;  // Stop if total_buffer size limit is reached
                    }
                }
            }

            printf("Accessible paths:\n%s\n", total_buffer);
            packet data_packet;
            data_packet.type = 8;
            strncpy(data_packet.data, total_buffer, sizeof(data_packet.data) - 1);
            data_packet.data[sizeof(data_packet.data) - 1] = '\0'; // Ensure null-termination
            if (send(client_socket, &data_packet, sizeof(data_packet), 0) < 0) {
                perror("Send failed");
            }

            // Send end packet to client
            packet end_packet;
            end_packet.type = 20;
            strcpy(end_packet.data, "End of accessible paths");
            if (send(client_socket, &end_packet, sizeof(end_packet), 0) < 0) {
                perror("Send failed");
            }
            // Send accessible paths to client
            char to_send[10] = "stop";
            if (send(client_socket, to_send, sizeof(to_send), 0) < 0) {
                perror("Send failed");
            }
            packet response_packet_cl;
            response_packet_cl.type = 9;
            Log(1, index, client_socket, response_packet_cl.type, response_packet_cl.data);
        }
        
         if(Packet.type==9){
            printf("Received copy request for file/folder: %s\n", Packet.data);
            pthread_mutex_lock(&lock);
            char temp_data[sizeof(Packet.data)+1];
            strcpy(temp_data, Packet.data);
            char *token = strtok(temp_data, "|");
            char *src = token;
            token = strtok(NULL, "|");
            char *dest = token;
            int file_found_src = 0;
            int file_found_dest = 0;
            int index_src = -1;
            int index_dest = -1;
            IpPortPair temp_src = get(&cache, src);
            if (temp_src.port_nm == -1) {
                for (int i = 0; i < ss_count; i++) {
                    if (search_path(ss_list[i].root, src) != -1) {
                        file_found_src = 1;
                        index_src = i;
                        break;
                    }
                }
                if(file_found_src==1){
                put(&cache, src, ss_list[index_src].ip, ss_list[index_src].port_client, ss_list[index_src].port_nm, index_src);
                temp_src = get(&cache, src);
                }
            } 
            else{
                file_found_src=1;
            }
            //print_cache(&cache);
            IpPortPair temp_dest = get(&cache, dest);
            if (temp_dest.port_nm == -1) {
                for (int i = 0; i < ss_count; i++) {
                    if (search_path(ss_list[i].root, dest) != -1) {
                        file_found_dest = 1;
                        index_dest = i;
                        break;
                    }
                }
                if(file_found_dest==1){
                put(&cache, dest, ss_list[index_dest].ip, ss_list[index_dest].port_client, ss_list[index_dest].port_nm,index_dest);
                temp_dest = get(&cache, dest);
                }
            } 
            else{
                file_found_dest = 1;
            }
            //print_cache(&cache);   
            
            ErrorCode res2;

            packet response_packet;
            if(file_found_src && file_found_dest){
               
                // if((strstr(src,".txt")!=NULL||strstr(src,".c")!=NULL||strstr(src,".mp3")!=NULL||strstr(src, ".h") != NULL)&& (strstr(dest,".txt")!=NULL||strstr(dest,".c")!=NULL||strstr(dest,".mp3")!=NULL||strstr(src, ".h")!= NULL)){
                //     int ss_socket = connect_server(temp_src.ip, temp_src.port_nm);
                //     response_packet.type=10;
                //     strcpy(response_packet.data,src);
                //     printf("%s %d\n",response_packet.data,response_packet.type);
                //     if (send(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                //         perror("Send failed");
                //         close(ss_socket);
                        
                //     }

                //     if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                //         perror("Receive failed");
                //         res2=ERR_OPERATION_FAILURE_COPY;
                //         close(ss_socket);

                //     }
                //     close(ss_socket);
                //     //char file[MAX_DATA_LENGTH] = "";
                //     // char *token3 = strtok(src, "/");
                //     // while (token3 != NULL) {
                //     //     // Store the current token in file
                //     //     strcpy(file, token3);
                //     //     // Get the next token
                //     //     token3 = strtok(NULL, "/");
                //     // }
                    
                //     // Print the result
                //     //printf("File: %s\n", file);
                // // printf("Response packet type: %s\n", response_packet.data);
                //     if(response_packet.type==100){
                //         printf("%s %d\n",temp_dest.ip,temp_dest.port_nm);
                //         int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
                //         response_packet.type=11;
                //         strcat(response_packet.data, "|");
                //         strcat(response_packet.data, dest);
                //         // strcat(response_packet.data, "/");
                //         // strcat(response_packet.data, file);
                //         printf("%s",response_packet.data);
                //         if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                //             perror("Send failed");
                //             res2=ERR_OPERATION_FAILURE_PASTE;
                //             close(dest_socket);
                //         }
                        
                //         if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                //             perror("Receive failed");
                //             res2=ERR_OPERATION_FAILURE_PASTE;
                //             close(dest_socket);
                //         }
                //         if(response_packet.type==100){
                //             res2=SUCCESS_OK;
                //         }
                //         else{
                //             res2=ERR_OPERATION_FAILURE_PASTE;
                //         }
                //         close(dest_socket);
                //     }
                //     else{
                //         res2=ERR_OPERATION_FAILURE_COPY;
                //     }
                // }
                 if (strstr(src, ".txt") != NULL || strstr(src, ".c") != NULL || strstr(src ,".mp3") != NULL||strstr(src, ".h") != NULL) {

                     int ss_socket = connect_server(temp_src.ip, temp_src.port_nm);
                
                    response_packet.type=10;
                    strcpy(response_packet.data,src);
                   // printf("%s %d\n",response_packet.data,response_packet.type);
                    if (send(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                        perror("Send failed");
                        res2=ERR_OPERATION_FAILURE_COPY;
                        close(ss_socket);
                        
                    }

                    if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                        perror("Receive failed");
                        res2=ERR_OPERATION_FAILURE_COPY;
                        close(ss_socket);
                    }
                    close(ss_socket);
                    char file[MAX_DATA_LENGTH] = "";
                    char *token3 = strtok(src, "/");
                    while (token3 != NULL) {
                        // Store the current token in file
                        strcpy(file, token3);
                        // Get the next token
                        token3 = strtok(NULL, "/");
                    }
                    printf("%s/n",file);
                    fflush(stdout);
                    if(response_packet.type==100){
                        int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
                        response_packet.type=11;
                    
                    strcat(response_packet.data, "|");
                    strcat(response_packet.data, dest);
                    strcat(response_packet.data, "/");
                    strcat(response_packet.data, file);
                    printf("Response packet type: %s\n", response_packet.data);
                    fflush(stdout);
                        if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                            perror("Send failed");
                            res2=ERR_OPERATION_FAILURE_PASTE;
                            close(dest_socket);
                        }
                        
                        if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                            perror("Receive failed");
                            res2=ERR_OPERATION_FAILURE_PASTE;
                            close(dest_socket);
                        }
                        if(response_packet.type==100){
                            res2=SUCCESS_OK;
                        }
                        else{
                            res2=ERR_OPERATION_FAILURE_PASTE;
                        }
                        close(dest_socket);
                    }
                    else{
                        res2=ERR_OPERATION_FAILURE_PASTE;
                    }
                }
                else{
                    //copy folder
                    char paths2[256][MAX_DATA_LENGTH];
                    int cnt;
                    add_paths_to_array(paths2, &cnt, ss_list[index_src].root);
                    print_paths(ss_list[index_src].root);
                    printf("Paths: %d\n", cnt);
                    for (int i = 0; i < cnt; i++) {
                        printf("%s\n", paths2[i]);
                    }
                    char paths[256][MAX_DATA_LENGTH];
                    int filtercnt = 0;
                    for (int i = 0; i < cnt; i++) {
                        if (strstr(paths2[i], src) != NULL) {
                            strcpy(paths[filtercnt], paths2[i]);
                            filtercnt++;
                        }
                    }
                        for (int i = 0; i < filtercnt; i++) {
                        if (strstr(paths[i], ".txt") != NULL || strstr(paths[i], ".c") != NULL || strstr(paths[i], ".mp3") != NULL||strstr(paths[i], ".h") != NULL) {
                            int ss_socket = connect_server(temp_src.ip, temp_src.port_nm);
                        
                            response_packet.type=10;
                            strcpy(response_packet.data,paths[i]);
                           // printf("%s %d\n",response_packet.data,response_packet.type);
                            if (send(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                                perror("Send failed");
                                close(ss_socket);
                                
                            }

                            if (recv(ss_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                                perror("Receive failed");
                                res2=ERR_OPERATION_FAILURE_COPY;
                                close(ss_socket);
                            }
                            close(ss_socket);
                            if(response_packet.type==100){
                                char arr5[1024];
                                int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
                                response_packet.type=11;
                            strcat(response_packet.data, "|");
                            strcat(response_packet.data, dest);
                            strcpy(arr5,dest);
                            char *path = paths[i];
                            if (path[0] == '.') {
                                path++; // Skip the initial dot
                            }
                            strcat(response_packet.data, path);
                            strcat(arr5,path);
                           // printf("Response packet type: %s\n", response_packet.data);
                                if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                                    perror("Send failed");
                                    res2=ERR_OPERATION_FAILURE_PASTE;
                                    close(dest_socket);
                                }
                                
                                if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                                    perror("Receive failed");
                                    res2=ERR_OPERATION_FAILURE_PASTE;
                                    close(dest_socket);
                                }
                                if(response_packet.type==100){
                                    res2=SUCCESS_OK;
                                    insert_path(ss_list[temp_dest.idx].root, arr5,temp_dest.idx);
                                }
                                else{
                                    res2=ERR_OPERATION_FAILURE_PASTE;
                                }
                                close(dest_socket);
                            }
                            else{
                                res2=ERR_OPERATION_FAILURE_PASTE;
                            }
                        }
                        else{
                            //create folder
                            char arr4[1024];
                            int dest_socket= connect_server(temp_dest.ip, temp_dest.port_nm);
                            memset(response_packet.data, 0, sizeof(response_packet.data));
                            strcat(response_packet.data, dest);
                           char *path = paths[i];
                            if (path[0] == '.') {
                                path++; // Skip the initial dot
                            }
                            strcat(response_packet.data, path);
                            strcpy(arr4,response_packet.data);
                            response_packet.type=13;
                            if (send(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                                perror("Send failed");
                                res2=ERR_OPERATION_FAILURE_CREATE;
                                close(dest_socket);
                            }
                            memset(response_packet.data, 0, sizeof(response_packet.data));
                            if (recv(dest_socket, &response_packet, sizeof(response_packet), 0) < 0) {
                                perror("Receive failed");
                                res2=ERR_OPERATION_FAILURE_CREATE;
                                close(dest_socket);
                                
                            }
                            res2 = response_packet.type;
                            if(response_packet.type==100){
                                //done successfully
                                insert_path(ss_list[temp_dest.idx].root, arr4,temp_dest.idx);

                            }
                            close(dest_socket);
                            }
                        }
                    }
                
            }
            else{
                res2 = ERR_NOT_FOUND_FILE;
            }
            
            // After creating the file/folder, you can send a response packet back to the client
            packet response_packet_cl;
            response_packet_cl.type = res2;
            strcpy(response_packet_cl.data, response_packet.data);
            printf("Response packet type: %s\n", response_packet_cl.data);
            if (send(client_socket, &response_packet_cl, sizeof(response_packet_cl), 0) < 0) {
                perror("Send failed");
            }
            Log(1, index, client_socket, response_packet_cl.type, response_packet_cl.data);
            pthread_mutex_unlock(&lock);
        }
        // pthread_mutex_unlock(&lock3);
    }

    // close(client_socket);
    return NULL;
}

// Thread functions for client and storage server handling
void *listenForClients(void *arg) {
    int client_fd = *(int *)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Listening for clients on port %d...\n", NM_CLIENT_PORT);
    while (1) {
        int new_client = accept(client_fd, (struct sockaddr *)&client_addr, &client_len);
        if (new_client < 0) {
            perror("Client connection failed");
            continue;
        }
        // RECIVED REQUEST
        printf("Connected to client: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client_operations, (void *)&new_client) < 0) {
            perror("Could not create thread");
            close(new_client);
        }
        // JOIN
        pthread_join(client_thread, NULL);

        // SUS
        close(new_client); // Close client connection after handling it
    }
    return NULL;
}

pthread_t temp;

void *listenForStorageServers(void *arg) {
    int ss_fd = *(int *)arg;
    struct sockaddr_in ss_addr;
    socklen_t ss_len = sizeof(ss_addr);

    printf("Listening for storage servers on port %d...\n", NM_SS_PORT_LISTEN);
    while (1) {
        int new_ss = accept(ss_fd, (struct sockaddr *)&ss_addr, &ss_len);
        if (new_ss < 0) {
            perror("Storage Server connection failed");
            continue;
        }
        printf("Connected to Storage Server: %s:%d\n", inet_ntoa(ss_addr.sin_addr), ntohs(ss_addr.sin_port));

        struct ip_and_socket ss_info;
        char ip_buffer[INET_ADDRSTRLEN];
        strncpy(ip_buffer, inet_ntoa(ss_addr.sin_addr), INET_ADDRSTRLEN);
        ip_buffer[INET_ADDRSTRLEN - 1] = '\0'; 
        ss_info.socket = new_ss;
        strcpy(ss_info.ip, ip_buffer);

        if (pthread_create(&temp, NULL, handle_ss_registration, (void *)&ss_info) < 0) {
            perror("Could not create thread");
            close(new_ss);
        }
        pthread_detach(temp); // Detach the thread to handle its own cleanup
        // JOIN
        
    }
    return NULL;
}

pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void *checkConnection(void *arg)
{
    while (1) 
    {
        pthread_mutex_lock(&lock2);
        int ss_sock;
        struct sockaddr_in serv_addr;
        // printf("inisde check\n");
        for (int i = 0; i < ss_count; i++)
        {
            // printf("hi %d", i);
            if (b_list->status == 1) {
                ss_sock = socket(AF_INET, SOCK_STREAM, 0);
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(ss_list[i].port_nm);

                if (inet_pton(AF_INET, ss_list[i].ip, &serv_addr.sin_addr) <= 0)
                {
                    perror("Invalid address or address not supported");
                    exit(EXIT_FAILURE);
                }
                if (ss_sock < 0)
                {
                    perror("Socket creation failed");
                    exit(1);
                }
                if (connect((ss_sock), (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                {
                    printf("Storage server port: %d, IP: %s got disconnected", ss_list[i].port_nm, ss_list[i].ip);
                    b_list[i].status = 0;
                }
                else
                {
                    // packet p;
                    // p.type = 101;
                    // send(ss_sock, &p, sizeof(p), 0);
                }
                close(ss_sock);
            }
        }
        sleep(10);
        pthread_mutex_unlock(&lock2);
    }
    
}

void *createBackup(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        printf("helllloooo\n");
        for (int i = 0; i < ss_count; i++) {
            //printf("test1\n");
            if (b_list[i].status == 1 && ss_count > 1) {
                if (b_list[i].is_backup0 != 1) {
                    b_list[i].backups[0] = backup_search(i, -1);
                    printf("Backup search: %d\n", b_list[i].backups[0]);
                    printf("%d %d\n", i, b_list[i].backups[0]);
                    ErrorCode res = func(i, b_list[i].backups[0]);
                    if (res == SUCCESS_OK) {
                        b_list[i].is_backup0 = 1;
                        printf("Backup created successfully for server %d\n", i);
                    } else {
                        b_list[i].is_backup0 = 0;
                        printf("Backup failed for server %d\n", i);
                    }
                }
                // if (b_list[i].is_backup1 != 1 && ss_count > 2) {
                //     b_list[i].backups[1] = backup_search(i, b_list[i].backups[0]);
                //     printf("Backup search: %d\n", b_list[i].backups[1]);
                //     ErrorCode res = func(i, b_list[i].backups[1]);
                //     if (res == SUCCESS_OK) {
                //         b_list[i].is_backup1 = 1;
                //         printf("Backup created successfully for server %d\n", i);
                //     } else {
                //         b_list[i].is_backup1 = 0;
                //        printf("Backup failed for server %d\n", i);
                //     }
                // }
            }
        }
        pthread_mutex_unlock(&lock);
        sleep(10); // Sleep for a while before checking again
    }
    return NULL;
}

int main() {
    signal(SIGQUIT, printLog);
    int NM_client_fd, NM_SS_fd, check_connection_fd;
    struct sockaddr_in NM_client_address, NM_SS_address;

    init_cache(&cache);
    // Open log file
    FILE *log_file = fopen("log.txt", "w");
    if (log_file == NULL) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    // Create sockets for client and storage server connections
    if ((NM_client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Client socket creation failed");
        exit(EXIT_FAILURE);
    }
    if ((NM_SS_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Storage Server socket creation failed");
        close(NM_client_fd);
        exit(EXIT_FAILURE);
    }

    // Set up address for client connection
    NM_client_address.sin_family = AF_INET;
    NM_client_address.sin_addr.s_addr = INADDR_ANY;
    NM_client_address.sin_port = htons(NM_CLIENT_PORT);

    // Set up address for storage server connection
    NM_SS_address.sin_family = AF_INET;
    NM_SS_address.sin_addr.s_addr = INADDR_ANY;
    NM_SS_address.sin_port = htons(NM_SS_PORT_LISTEN);

    // Bind client socket
    if (bind(NM_client_fd, (struct sockaddr *)&NM_client_address, sizeof(NM_client_address)) < 0) {
        perror("Binding client socket failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }

    // Bind storage server socket
    if (bind(NM_SS_fd, (struct sockaddr *)&NM_SS_address, sizeof(NM_SS_address)) < 0) {
        perror("Binding storage server socket failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening on client and storage server sockets
    if (listen(NM_client_fd, MAX_CONNECTIONS) < 0) {
        perror("Listening on client socket failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(NM_SS_fd, MAX_CONNECTIONS) < 0) {
        perror("Listening on storage server socket failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }

    
    pthread_t client_thread, ss_thread, backup_thread, check_connection_thread;
    if (pthread_create(&client_thread, NULL, listenForClients, (void *)&NM_client_fd) != 0) {
        perror("Client thread creation failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&ss_thread, NULL, listenForStorageServers, (void *)&NM_SS_fd) != 0) {
        perror("Storage server thread creation failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&check_connection_thread, NULL, checkConnection, NULL) != 0)
    {
        perror("Backup thread creation failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&backup_thread, NULL, createBackup, NULL) != 0)
    {
        perror("Backup thread creation failed");
        close(NM_client_fd);
        close(NM_SS_fd);
        exit(EXIT_FAILURE);
    }
    
    // Join threads to keep the main process running
    pthread_join(client_thread, NULL);
    pthread_join(ss_thread, NULL);

    // Cleanup
    close(NM_client_fd);
    close(NM_SS_fd);

    return 0;
}
