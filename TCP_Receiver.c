#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    

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
    if (setsockopt(server_socket, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo)) < 0) {
        perror("Error setting congestion control algorithm");
        exit(1);
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
    
    while (1)
    {
        /* code */
    
    
    FILE *file = fopen("received_file.txt", "wb");
    if (file == NULL) {
        perror("Error creating file");
        exit(1);
    }

    ssize_t bytes_received;
    do {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Error receiving data");
            exit(1);
        }

        if (bytes_received > 0) {
            fwrite(buffer, 1, bytes_received, file);
        }
    } while (bytes_received > 0 && strncmp(buffer, "exit", 4) != 0);

    fclose(file);
    if(strncmp(buffer,"exit", 4) == 0){
        break;
    }
}
    printf("File received and saved as received_file.txt\n");

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}