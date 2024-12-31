#include "helper/SS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#define BACKLOG 5 // Number of pending connections allowed in the queue

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
        perror("Invalid address or address not supported");
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

ErrorCode resolvePath(char *inputPath, size_t inputPathSize) {
    char resolved_path[4096];
    if (realpath(inputPath, resolved_path) == NULL) {
        perror("Path resolution failed");
        return ERR_OPERATION_FAILURE_READ;
    }
    strcpy(inputPath, resolved_path);
    return SUCCESS_OK;
}

void *handleClientRequest(void *client_sock) {
    int client_socket = *(int *)client_sock;
    packet Packet;
    int read_size, index = -1;

    while ((read_size = recv(client_socket, &Packet, sizeof(Packet), 0)) > 0) {
        if (Packet.type == 1) {
            printf("Received READ request for file: %s\n", Packet.data);    
            ErrorCode res2;
            char resolved_path[4096];
            if (realpath(Packet.data, resolved_path) == NULL) {
                perror("Path resolution failed");
                res2 = ERR_OPERATION_FAILURE_READ;
                packet response;
                response.type = res2;
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;// Exit the program or handle the error appropriately
            }
            printf("Resolved path: %s\n", resolved_path);
            FILE *file = fopen(resolved_path, "rb");
            if (file == NULL) {
                perror("File open failed");
                res2=ERR_OPERATION_FAILURE_READ;
                 packet response;
                response.type = res2;
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }

            packet response;
            while ((read_size = fread(response.data, 1, sizeof(response.data), file)) > 0) {
                response.type = SUCCESS_OK;
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                    break;
                }
            }
            // Send a packet of type 20 to indicate end of file transfer
            packet end_packet;
            end_packet.type = 20;
            memset(end_packet.data, 0, sizeof(end_packet.data)); // Clear the data field

            if (send(client_socket, &end_packet, sizeof(end_packet), 0) < 0) {
                perror("Send failed");
            }
            fclose(file);
               
        } else if (Packet.type == 2) {
            printf("Received WRITE request for file: %s\n", Packet.data);
            char *token = strtok(Packet.data, "|");
            char *filename = token;
            token = strtok(NULL, "|");
            char *file_content = token;
            ErrorCode res2;
            char resolved_path[4096];
            printf("karthik debug\n");
            if (realpath(filename, resolved_path) == NULL) {
                perror("Path resolution failed");
                res2 = ERR_OPERATION_FAILURE_READ;
                packet response;
                response.type = res2;
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
               break;
            }
            printf("Resolved path: %s\n", resolved_path);
            
            FILE *file = fopen(resolved_path, "wb");
            if (file == NULL) {
                 perror("File open failed");
                res2=ERR_OPERATION_FAILURE_WRITE;
                 packet response;
                response.type = res2;
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }

            if (fwrite(file_content, sizeof(char), strlen(file_content), file) < strlen(file_content)) {
                perror("File write failed");
                res2 = ERR_OPERATION_FAILURE_WRITE;
                packet response;
                response.type = res2;
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }
            packet response;
            response.type = 100; // Custom type for file write started
            strncpy(response.data, "File write ended", sizeof(response.data) - 1);
            response.data[sizeof(response.data) - 1] = '\0'; // Ensure null-termination
            if (send(client_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }

            packet stop_packet;
            stop_packet.type = 20;
            memset(stop_packet.data, 0, sizeof(stop_packet.data)); // Clear the data field
            if (send(client_socket, &stop_packet, sizeof(stop_packet), 0) < 0) {
                perror("Send failed");
            }
            fclose(file);
        } else if (Packet.type == 3 || Packet.type == 30) {
            ErrorCode res2;
            char resolved_path[4096];
            if (Packet.type == 3) {
                printf("Received WRITE request for file: %s\n", Packet.data);
                char *filename = Packet.data;
                printf("karthik debug\n");
                if (realpath(filename, resolved_path) == NULL) {
                    perror("Path resolution failed");
                    res2 = ERR_OPERATION_FAILURE_READ;
                    packet response;
                    response.type = res2;
                    if (send(client_socket, &response, sizeof(response), 0) < 0) {
                        perror("Send failed");
                    }
                break;
                }
                printf("Resolved path: %s\n", resolved_path);
            } else {
                printf("inside the async\n");
                FILE *file = fopen(resolved_path, "wb");
                if (file == NULL) {
                    perror("File open failed");
                    res2=ERR_OPERATION_FAILURE_WRITE;
                    packet response;
                    response.type = res2;
                    if (send(client_socket, &response, sizeof(response), 0) < 0) {
                        perror("Send failed");
                    }
                    break;
                }
                packet response;
                response.type = 100; // Custom type for file write started
                strncpy(response.data, "File write started", sizeof(response.data) - 1);
                response.data[sizeof(response.data) - 1] = '\0'; // Ensure null-termination
                if (send(client_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                pthread_mutex_t lock;
                pthread_mutex_init(&lock, NULL);

                pthread_mutex_lock(&lock);
                while (1) {
                    packet data_packet;
                    if ((read_size = recv(client_socket, &data_packet, sizeof(data_packet), 0)) <= 0) {
                        if (read_size == 0) {
                            printf("Client disconnected\n");
                        } else {
                            perror("Receive failed");
                        }
                        break;
                    }

                    if (data_packet.type == 25) {
                        // printf("Received end of file transfer signal\n");
                        break;
                    }

                    size_t data_size = strnlen(data_packet.data, sizeof(data_packet.data));
                    if (fwrite(data_packet.data, 1, data_size, file) < data_size) {
                        perror("File write failed");
                        break;
                    }
                    sleep(1);
                }
                pthread_mutex_unlock(&lock);
                pthread_mutex_destroy(&lock);

                // Send a message to the client indicating that file write has ended
                packet end_response;
                end_response.type = 100; // Custom type for file write ended
                strncpy(end_response.data, "File write ended", sizeof(end_response.data) - 1);
                end_response.data[sizeof(end_response.data) - 1] = '\0'; // Ensure null-termination
                if (send(client_socket, &end_response, sizeof(end_response), 0) < 0) {
                    perror("Send failed");
                }

                // Send a packet of type 20 to indicate stop
                packet stop_packet;
                stop_packet.type = 20;
                memset(stop_packet.data, 0, sizeof(stop_packet.data)); // Clear the data field
                if (send(client_socket, &stop_packet, sizeof(stop_packet), 0) < 0) {
                    perror("Send failed");
                }
                fclose(file);
            }
        }
    }

    return NULL;
}

void* handleNamingServerRequest(void* ns_sock) {
    int ns_socket = *(int *)ns_sock;
    packet Packet;
    int read_size, index = -1;

    while ((read_size = recv(ns_socket, &Packet, sizeof(Packet), 0)) > 0) {
        printf("Received packet type: %d\n", Packet.type);
        ErrorCode res2;
        if(Packet.type == 101){
            return NULL;
        }
        if(Packet.type == 4){
            printf(" Storage Received CREATE request for file/folder: %s\n", Packet.data);
            //pthread_mutex_lock(&lock);
            
            char *token = strtok(Packet.data, "|");
            char *path2 = token;
            token = strtok(NULL, "|");
            char *file_to_add = token;
            char resolved_path[4096];
            if (realpath(path2, resolved_path) == NULL) {
                perror("Path resolution failed");
                break;
            }
            
            char *path=resolved_path;
            
            // Check if the path is valid
            if (access(path, F_OK) != -1) {
                // Path exists, create the file
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", path, file_to_add);
                FILE *file = fopen(full_path, "w");
                if (file) {
                    printf("File created: %s\n", full_path);
                    res2 = SUCCESS_OK;
                    fclose(file);
                } else {
                    res2 = ERR_OPERATION_FAILURE_CREATE;
                }
            } else {
                printf("Invalid path: %s\n", path);
                res2 = ERR_NOT_FOUND_FILE;
            }
            // Send response to the naming server
            packet response;
            response.type = res2; // Response type for file creation
            
            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }
            

            //pthread_mutex_unlock(&lock);
        } 
        else if(Packet.type==6){
            printf(" Storage Received DELETE request for file/folder: %s\n", Packet.data);
            struct stat path_stat;
             char resolved_path[4096];
            if (realpath(Packet.data, resolved_path) == NULL) {
                perror("Path resolution failed");
                break;
            }
            char *path=resolved_path;
             ErrorCode res2;
    // Check if the path exists
            if (stat(path, &path_stat) == 0) {
               
                // Check if it's a regular file
                if (S_ISREG(path_stat.st_mode)) {
                    if (remove(path) == 0) {
                        printf("File at path '%s' deleted successfully.\n", path);
                        res2 = SUCCESS_OK;
                        
                    } else {
                        perror("Error deleting file");
                        res2 = ERR_OPERATION_FAILURE_DELETE;
                        
                    }
                }
                // Check if it's a directory
                else if (S_ISDIR(path_stat.st_mode)) {
                    if (rmdir(path) == 0) {
                        printf("Directory at path '%s' deleted successfully.\n", path);
                        res2 = SUCCESS_OK;
                        
                    } else {
                        perror("Error deleting directory");
                        res2 = ERR_OPERATION_FAILURE_DELETE;
                        
                    }
                } else {
                    printf("Path exists but is neither a file nor a directory.\n");
                    res2 = ERR_OPERATION_FAILURE_DELETE;
                }
            } else {
                printf("Path '%s' does not exist.\n", path);
                res2 = ERR_NOT_FOUND_FILE;
            }
            packet response;
             response.type = res2; // Response type for file creation
            printf("Response type: %d\n", response.type);
            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }
        }
        else if (Packet.type==5) {
        printf(" Storage Received CREATEFOLDER request for file/folder: %s\n", Packet.data);
        //pthread_mutex_lock(&lock);
        ErrorCode res2;
        char *token = strtok(Packet.data, "|");
        char *path2 = token;
        token = strtok(NULL, "|");
        char *file_to_add = token;
         char resolved_path[4096];
            if (realpath(path2, resolved_path) == NULL) {
                perror("Path resolution failed");
                break;
            }
            
            char *path=resolved_path;
        // Check if the path is valid
        if (access(path, F_OK) != -1) {
            // Path exists, create the file
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, file_to_add);
            if (mkdir(full_path, 0777) == 0) {
                printf("Folder created: %s\n", full_path);
                res2 = SUCCESS_OK;
            } else {
                perror("Folder creation failed");
                res2 = ERR_OPERATION_FAILURE_CREATE;
            }
        } else {
            printf("Invalid path: %s\n", path);
            res2 = ERR_NOT_FOUND_FILE;
        }
        packet response;
            response.type = res2; // Response type for file creation
            
            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }

        } else if (Packet.type == 7) {
            printf("Received INFO request for file: %s\n", Packet.data);
             char resolved_path[4096];
            if (realpath(Packet.data, resolved_path) == NULL) {
                perror("Path resolution failed");
                 res2 = ERR_OPERATION_FAILURE_INFO;
                packet response;
                response.type = res2;
                if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }
            ErrorCode res2;
            char *path=resolved_path;
            printf("Resolved path: %s\n", path);
            struct stat file_stat;
            if (stat(path, &file_stat) == -1) {
                perror("File stat failed");
                 res2 = ERR_OPERATION_FAILURE_INFO;
                packet response;
                response.type = res2;
                if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }

            char response_data[MAX_SIZE];
            snprintf(response_data, sizeof(response_data), "Size: %ld bytes, Permissions: %o", file_stat.st_size, file_stat.st_mode & 0777);
            
            // Create the permissions string
            char permissions[11];
            snprintf(permissions, sizeof(permissions), "%c%c%c%c%c%c%c%c%c%c",
                    (S_ISDIR(file_stat.st_mode)) ? 'd' : '-',
                    (file_stat.st_mode & S_IRUSR) ? 'r' : '-',
                    (file_stat.st_mode & S_IWUSR) ? 'w' : '-',
                    (file_stat.st_mode & S_IXUSR) ? 'x' : '-',
                    (file_stat.st_mode & S_IRGRP) ? 'r' : '-',
                    (file_stat.st_mode & S_IWGRP) ? 'w' : '-',
                    (file_stat.st_mode & S_IXGRP) ? 'x' : '-',
                    (file_stat.st_mode & S_IROTH) ? 'r' : '-',
                    (file_stat.st_mode & S_IWOTH) ? 'w' : '-',
                    (file_stat.st_mode & S_IXOTH) ? 'x' : '-');

            // Concatenate permissions string to response data
            strncat(response_data, ", Permissions: ", sizeof(response_data) - strlen(response_data) - 1);
            strncat(response_data, permissions, sizeof(response_data) - strlen(response_data) - 1);
            // Get the last modified time
            char time_str[100];
            struct tm *tm_info = localtime(&file_stat.st_mtime);
            strftime(time_str, sizeof(time_str), ", Last modified: %Y-%m-%d %H:%M:%S", tm_info);

            // Concatenate last modified time to response data
            strncat(response_data, time_str, sizeof(response_data) - strlen(response_data) - 1);

            // Prepare packet
            packet response;
            response.type = SUCCESS_OK;
            strncpy(response.data, response_data, sizeof(response.data) - 1);
            response.data[sizeof(response.data) - 1] = '\0';  // Ensure null-termination
            printf("Response data: %s\n", response.data);
            // Send packet
            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }
        }
         if (Packet.type == 10) {
           // printf("Received READ request for file: %s\n", Packet.data);    
            ErrorCode res2;
            char resolved_path[4096];
            if (realpath(Packet.data, resolved_path) == NULL) {
                perror("Path resolution failed");
                res2 = ERR_OPERATION_FAILURE_READ;
                packet response;
                response.type = res2;
                if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;// Exit the program or handle the error appropriately
            }
            printf("Resolved path: %s\n", resolved_path);
            FILE *file = fopen(resolved_path, "rb");
            if (file == NULL) {
                perror("File open failed");
                res2=ERR_OPERATION_FAILURE_READ;
                 packet response;
                response.type = res2;
                if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }

            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *file_buffer = malloc(file_size+1);
            if (file_buffer == NULL) {
                perror("Memory allocation failed");
                fclose(file);
                break;
            }

            fread(file_buffer, 1, file_size, file);
            file_buffer[file_size] = '\0';
            res2=SUCCESS_OK;
            fclose(file);
            packet response;
            response.type = res2; // Response type for file creation
            if (file_size < sizeof(response.data)) {
                strcpy(response.data, file_buffer);
            } else {
                memcpy(response.data, file_buffer, sizeof(response.data) - 1);
                response.data[sizeof(response.data) - 1] = '\0'; // Null-terminate the string
            }

            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }
            free(file_buffer);
            

            
        } else if (Packet.type == 11) {
            //printf("Received WRITE request for file: %s\n", Packet.data);
            char *token = strtok(Packet.data, "|");
            char *file_content = token;
            token = strtok(NULL, "|");
            char *filename = token;
            printf("Filename: %s\n", filename);
            
            ErrorCode res2;
            char resolved_path[4096];
            char cwd[1024];
             // Get the current working directory
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd error");
            break;
            }

        // Construct the absolute path
            snprintf(resolved_path, sizeof(resolved_path), "%s/%s", cwd, filename);

            
            printf("Resolved path: %s\n", resolved_path);
            
            FILE *file = fopen(resolved_path, "wb");
            if (file == NULL) {
                 perror("File open failed");
                res2=ERR_OPERATION_FAILURE_WRITE;
                 packet response;
                response.type = res2;
                if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                    perror("Send failed");
                }
                break;
            }
            if (fwrite(file_content, sizeof(char), strlen(file_content), file) < strlen(file_content)) {
                perror("File write failed");
            }
            fclose(file);
            packet response;
            res2=100;
            response.type = res2; 
            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }
        }
         else if (Packet.type==13) {
        //printf(" Storage Received CREATEFOLDER request for file/folder: %s\n", Packet.data);
        //pthread_mutex_lock(&lock);
        ErrorCode res2;
        char path2[1024];

        // Find the last occurrence of '/'
        char *last_slash = strrchr(Packet.data, '/');

        if (last_slash != NULL) {
            // Split the path and folder
            size_t path_length = last_slash - Packet.data;
            strncpy(path2, Packet.data, path_length);
            path2[path_length] = '\0'; // Null-terminate the path

            char *folder_to_add = last_slash + 1;

            printf("Path: %s\n", path2);
            printf("Folder to add: %s\n", folder_to_add);
         char resolved_path[4096];
           char cwd[1024];
             // Get the current working directory
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd error");
            break;
            }

        // Construct the absolute path
            snprintf(resolved_path, sizeof(resolved_path), "%s/%s", cwd, path2);
            
            char *path=resolved_path;
        // Check if the path is valid
        if (access(path, F_OK) != -1) {
            // Path exists, create the file
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, folder_to_add);
            if (mkdir(full_path, 0777) == 0) {
                printf("Folder created: %s\n", full_path);
                res2 = SUCCESS_OK;
            } else {
                perror("Folder creation failed");
                res2 = ERR_OPERATION_FAILURE_CREATE;
            }
        } else {
            printf("Invalid path: %s\n", path);
            res2 = ERR_NOT_FOUND_FILE;
        }
        }
        else{
            res2 = ERR_OPERATION_FAILURE_CREATE;
        }
        packet response;
            response.type = res2; // Response type for file creation
            
            if (send(ns_socket, &response, sizeof(response), 0) < 0) {
                perror("Send failed");
            }

        //pthread_mutex_unlock(&lock);
        }
    }

    //close(ns_socket);
    return NULL;
}
void *handleClient(void *client_sock) {
    int client_fd = *(int *)client_sock;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_client;

    while (1) {
        new_client = accept(client_fd, (struct sockaddr *)&client_addr, &client_len);
        pthread_t temp;
        if (new_client < 0) {
            perror("Client connection failed");
            continue;
        }
        printf("Connected to client: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Handle each client request in a separate thread
        if (pthread_create(&temp, NULL, handleClientRequest, (void *)&new_client) < 0) {
            perror("Could not create thread");
            close(new_client);
        }
    }
    return NULL;
}

void *handleNamingServer(void *ns_sock) {
    int ns_fd = *(int *)ns_sock;
    struct sockaddr_in ns_addr;
    socklen_t ns_len = sizeof(ns_addr);
    int new_ns;

    while (1) {
        new_ns = accept(ns_fd, (struct sockaddr *)&ns_addr, &ns_len);
        pthread_t temp;
        if (new_ns < 0) {
            perror("Naming Server connection failed");
            continue;
        }
        //printf("Connected to Naming fgadfgServer: %s:%d\n", inet_ntoa(ns_addr.sin_addr), ntohs(ns_addr.sin_port));

        // Handle each Naming Server request in a separate thread
        if (pthread_create(&temp, NULL, handleNamingServerRequest, (void *)&new_ns) < 0) {
            perror("Could not create thread");
            close(new_ns);
        }
        pthread_join(temp, NULL);
        // pthread_detach(temp); // Detach the thread to handle its own cleanup
    }
    return NULL;
}

int findport();

void initialize_storage_server(StorageServerInfo *ss_info, int port_nm, int port_client, char *paths, int num_paths, trie_node *root) {
    ss_info->port_nm = port_nm;
    ss_info->port_client = port_client;
    ss_info->num_paths = num_paths;
    strcpy(ss_info->accessible_paths, paths);
    ss_info->root = NULL;
}

void listFilesRecursively(char *basePath, char *relativePath, char **file_paths, int *file_count) {
    char path[1024];
    struct dirent *entry;
    DIR *dir = opendir(basePath);

    if (!dir) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);
                char relPath[1024];
                snprintf(relPath, sizeof(relPath), "%s/%s", relativePath, entry->d_name);
                // Add directory to the array
                char *dirpath = malloc(1024);
                snprintf(dirpath, 1024, "%s", relPath);
                file_paths[(*file_count)++] = dirpath;
                listFilesRecursively(path, relPath, file_paths, file_count);
            }
        } else if (entry->d_type == DT_REG) {
            char *filepath = malloc(1024);
            snprintf(filepath, 1024, "%s/%s", relativePath, entry->d_name);
            file_paths[(*file_count)++] = filepath;
        }
    }
    closedir(dir);
}



int main() {
    int namingserver = NSPORT;  // Port to communicate with the Naming Server
    int ss_client_port = 5001;
    int ss_ns_port = 5002;

    int server_to_NS, server_to_client;
    struct sockaddr_in NS_server_addr, client_server_addr;

    printf("Storage Server to naming server port %d\n", namingserver);
    printf("Storage Server to client port %d\n", ss_client_port);
    

    server_to_NS = socket(AF_INET, SOCK_STREAM, 0);
    if (server_to_NS < 0) {
        perror("Socket creation failed for Naming Server");
        exit(EXIT_FAILURE);
    }
    
    // Configure address for Naming Server
    memset(&NS_server_addr, 0, sizeof(NS_server_addr));
    NS_server_addr.sin_family = AF_INET;
    NS_server_addr.sin_port = htons(namingserver);
    char naming_server_ip[INET_ADDRSTRLEN];

    printf("Enter the Naming Server IP: ");
    if (fgets(naming_server_ip, sizeof(naming_server_ip), stdin) == NULL) {
        perror("Failed to read Naming Server IP");
        close(server_to_NS);
        exit(EXIT_FAILURE);
    }

    naming_server_ip[strcspn(naming_server_ip, "\n")] = '\0'; // Remove newline character

    if (inet_pton(AF_INET, naming_server_ip, &NS_server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(server_to_NS);
        exit(EXIT_FAILURE);
    }

    if (connect(server_to_NS, (struct sockaddr *)&NS_server_addr, sizeof(NS_server_addr)) < 0) {
        perror("Connection to Naming Server failed");
        close(server_to_NS);
        exit(EXIT_FAILURE);
    }

    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        close(server_to_NS);
        exit(EXIT_FAILURE);
    }

    // Send the current working directory to the Naming Servey
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(cwd)) == NULL) {
        perror("opendir() error");
        close(server_to_NS);
        exit(EXIT_FAILURE);
    }

    // Loop through all files in the directory and send their path names
    char *file_paths[4096];
    int file_count = 0;

    listFilesRecursively(cwd, ".", file_paths, &file_count);


    printf("Select the paths you want to share to NS:(Select the index number of paths 0-indexed)\n");

    // Print all file paths
    for (int i = 0; i < file_count; i++) {
        printf("File path: %s\n", file_paths[i]);
    }

    int selected_paths[4096];
    int selected_count = 0;
    char input[10];

    while (1) {
        printf("Enter the index of the path to share (or type 'done' to finish): ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        if (strncmp(input, "done", 4) == 0) {
            break;
        }
        int index = atoi(input);
        if (index >= 0 && index < file_count) {
            selected_paths[selected_count++] = index;
        } else {
            printf("Invalid index. Please try again.\n");
        }
    }

    StorageServerInfo ss_info;
    // ss_info.root = (trie_node*)(sizeof(trie_node));
    
    // if (ss_info.accessible_paths == NULL) {
    //     perror("Memory allocation for accessible_paths failed");
    //     close(server_to_NS);
    //     exit(EXIT_FAILURE);
    // }
    char selected_file_paths[4096];
    memset(selected_file_paths, 0, sizeof(selected_file_paths));
    for (int i = 0; i < selected_count; i++) {
        strcat(selected_file_paths, file_paths[selected_paths[i]]);
        if (i < selected_count - 1) {
            strcat(selected_file_paths, "|");
        }
    }

    // trie_node *root = create_trie_node();

    // if (root == NULL) {
    //     perror("Trie creation failed");
    //     close(server_to_NS);
    //     exit(EXIT_FAILURE);
    // }

    // printf("%d\n", root->end);
    // printf("%p\n", root);

    printf("%d\n", selected_count);
    initialize_storage_server(&ss_info, ss_ns_port, ss_client_port, selected_file_paths, selected_count, NULL);

    // if (ss_info.root == NULL) {
    //     perror("Memory allocation for root failed");
    //     close(server_to_NS);
    //     exit(EXIT_FAILURE);
    // }

    if (send(server_to_NS, &ss_info, sizeof(ss_info), 0) < 0) {
        perror("Failed to send StorageServerInfo to Naming Server");
        close(server_to_NS);
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < file_count; i++) {
        free(file_paths[i]);
    }
    
    // CREATE THREADS using the two ss_clients and ss_ns_port
    int ss_client_fd, ss_ns_fd;
    struct sockaddr_in ss_client_address, ss_ns_address;

    if ((ss_client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Client socket creation failed");
        exit(EXIT_FAILURE);
    }
    if ((ss_ns_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Storage Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up address for client connection
    memset(&ss_client_address, 0, sizeof(ss_client_address));
    ss_client_address.sin_family = AF_INET;
    ss_client_address.sin_addr.s_addr = INADDR_ANY;
    ss_client_address.sin_port = htons(ss_client_port);

    // Set up address for storage server connection
    memset(&ss_ns_address, 0, sizeof(ss_ns_address));
    ss_ns_address.sin_family = AF_INET;
    ss_ns_address.sin_addr.s_addr = INADDR_ANY;
    ss_ns_address.sin_port = htons(ss_ns_port);

    // Bind client socket
    if (bind(ss_client_fd, (struct sockaddr *)&ss_client_address, sizeof(ss_client_address)) < 0) {
        perror("Binding client socket failed");
        close(ss_client_fd);
        exit(EXIT_FAILURE);
    }

    // Bind storage server socket
    if (bind(ss_ns_fd, (struct sockaddr *)&ss_ns_address, sizeof(ss_ns_address)) < 0) {
        perror("Binding storage server socket failed");
        close(ss_ns_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening on client and storage server sockets
    if (listen(ss_client_fd, MAX_CONNECTIONS) < 0) {
        perror("Listening on client socket failed");
        exit(EXIT_FAILURE);
    }
    if (listen(ss_ns_fd, MAX_CONNECTIONS) < 0) {
        perror("Listening on storage server socket failed");
        exit(EXIT_FAILURE);
    }
    connectst conn;
    conn.ns=ss_ns_fd;
    conn.client=ss_client_fd;
    
    // Create threads to handle clients and storage servers
    pthread_t ns_thread, client_thread;
    if (pthread_create(&ns_thread, NULL, handleNamingServer, (void *)&ss_ns_fd) != 0) {
        perror("Client thread creation failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&client_thread, NULL, handleClient, (void *)&ss_client_fd) != 0) {
        perror("Storage server thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Join threads to keep the main process running
    pthread_join(client_thread, NULL);
    pthread_join(ns_thread, NULL);

    // Cleanup
    close(ss_ns_fd);
    close(ss_client_fd);
    return 0;
}
