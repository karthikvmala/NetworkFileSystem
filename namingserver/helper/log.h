#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void printLog(int signum) {
    if (signum == SIGQUIT) {
        // printf("hi\n");
        FILE *log_file = fopen("log.txt", "r");
        if (log_file == NULL) {
            fprintf(stderr, "ERROR OPENING \"log.txt\" \n");
            return;
        }

        char buffer[1024];
        int bytes_read;
        printf("\n=======LOG=======\n\n");
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), log_file)) > 0) {
            fwrite(buffer, 1, bytes_read, stdout);
        }
        printf("\n=================\n\n");

        fclose(log_file);
    }
}

// ss_or_client = 1 for SS, 0 for client
// ss_id = -1 if communication with client
int Log(int ss_or_client, int ss_id, int port, int req_type, char *req_data) {
    FILE *log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        fprintf(stderr, "ERROR OPENING \"log.txt\" \n");
        return 0;
    }

    if (ss_or_client == 1) {
        fprintf(log_file, "Communicating with SS : %d\n", ss_id);
        fprintf(log_file, "SS socket             : %d\n", port);
        fprintf(log_file, "Request type         : %d\n", req_type);
        fprintf(log_file, "Request data         : %s\n", req_data);
        fprintf(log_file, "\n");
    } else {
        fprintf(log_file, "Communicating with client\n");
        fprintf(log_file, "Client socket          : %d\n", port);
        fprintf(log_file, "Request type         : %d\n", req_type);
        fprintf(log_file, "Request data         : %s\n", req_data);
        fprintf(log_file, "\n");
    }

    fclose(log_file); // Ensure you close the file after writing
    printf("LOG DONE\n");
    return 1;
}
