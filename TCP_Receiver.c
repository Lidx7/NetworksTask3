#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>
#define BUFFER_SIZE 4096
#define FALSE 0
#define TRUE 1

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <algorithm>\n", argv[0]);
        return 1;
    }
    // transform the information that we get and declaring all the variabels and objects
    int port = atoi(argv[1]);
    char* algo = argv[2];

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Set congestion control algorithm
    if (strcmp(algo, "reno") == 0 || strcmp(algo, "cubic") == 0) {
        if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo) + 1) < 0) {
            printf("Error setting congestion control algorithm\n");
            return 1;
        }
    } else {
        printf("Invalid algorithm\n");
        return 1;
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 1) < 0) {
        perror("Error listening on socket");
        exit(1);
    }

    printf("Server is listening on port %d...\n", port);

    // Accept incoming connection
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0) {
        perror("Error accepting connection");
        exit(1);
    }

    printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Receive file data
    int repeat_counter = 0;
    char name[50];
    sprintf(name, "received_file%d.txt", repeat_counter);
    FILE *file = fopen(name, "wb");
    if (file == NULL) {
        perror("Error creating file");
        exit(1);
    }
    // the time meassure
    clock_t start_time, end_time;
    double total_time;
    start_time = clock();   

    ssize_t total_bytes_received = 0;
    ssize_t bytes_received;
    // the recivieng process
    // were receveing the buffers one by one and writing them down on the new file 
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        total_bytes_received += bytes_received;

        if (bytes_received < 0) {
            perror("Error receiving data");
            exit(1);
        }

        if (strstr(buffer, "\exit") != NULL) {
            char* exit_position = strstr(buffer, "\exit");
            size_t bytes_to_write = exit_position - buffer;
            fwrite(buffer, 1, bytes_to_write, file);
            fclose(file);

            end_time = clock();
            total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            printf("File received and saved as %s (Time taken: %.8f seconds)\n", name, total_time);
            
            double average_bandwidth = (total_bytes_received * 8) / (total_time * 1024 * 1024); // in Mbps
            printf("Average Bandwidth: %.2f Mbps\n", average_bandwidth);

            repeat_counter++;
            sprintf(name, "received_file%d.txt", repeat_counter); 
            file = fopen(name, "wb");
            if (file == NULL) {
                perror("Error creating file");
                exit(1);
            }
            
            fwrite(exit_position + strlen("\exit") + 1, 1, bytes_received - bytes_to_write - strlen("\exit") - 1, file);
            start_time = clock();
            continue;
        } else {
            fwrite(buffer, 1, bytes_received, file);
        }
    } 
    // closing the file 
    fclose(file);

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
