#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include "RandomFileGen.c"
#include <netinet/tcp.h>

#define RENO "reno"
#define CUBIC "cubic"
#define MIN_SIZE 2097152
#define FILE_SIZE 2000000 

// Helper function to send data to the receiver
int sendData(int socketfd, void* buffer, int len) {
    int sentd = send(socketfd, buffer, len, 0);
    if (sentd == -1) {
        perror("send");
        exit(1);
    }
    else if (!sentd) {
        printf("Receiver doesn't accept requests.\n");
    }
    else if (sentd < len) {
        printf("Data was only partly sent (%d/%d bytes).\n", sentd, len);
    }
    else {
        printf("Total bytes sent is %d.\n", sentd);
    }

    return sentd;
}

int main(int argc, char* argv[]) {
    // correct argument input check
    if (argc < 4) {
        printf("Usage: %s <IP> <port> <algorithm>\n", argv[0]);
        return 1;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);
    char* algo = argv[3];

    //enter desiered size of the random file
    int file_size;
    do{
        printf("Enter the size of the file to be sent (minimum size is 2MB): ");
        scanf("%d", &file_size);
    } while(file_size < MIN_SIZE);
    char* rand_file = util_generate_random_data(file_size);

    //creating socket
    struct sockaddr_in serverAddress;
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket < 0) {
        printf("Error in creating socket\n");
        return 1;
    }

    //setting the congestion control algorithm
    if (strcmp(algo, "reno") == 0 || strcmp(algo, "cubic") == 0) {
        if (setsockopt(network_socket, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo) + 1) < 0) {
            printf("Error setting congestion control algorithm\n");
            return 1;
        }
    }
    else {
        printf("Invalid algorithm\n");
        return 1;
    }

    //binding the socket to the port and ip
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip);

    //connecting
    if (connect(network_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("Error in connecting to server\n");
        return 1;
    }

    //sending the file and repeating as long as the user wants
    char again;
    do {
        sendData(network_socket, rand_file, file_size);
        send(network_socket, "\exit", strlen("\exit") + 1, 0);
        printf("Do you want to send the file again? type y for yes, any other character for no\n");
        scanf(" %c", &again);
    } while (again == 'y' || again == 'Y');

    //closing the socket and freeing the memory
    close(network_socket);
    free(rand_file);

    return 0;
}