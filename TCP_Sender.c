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

int sendData(int socketfd, void* buffer, int len) {
    int sentd = send(socketfd, buffer, len, 0);

    if (sentd == -1) {
        perror("send");
        exit(1);
    } else if (!sentd) {
        printf("Receiver doesn't accept requests.\n");
    } else if (sentd < len) {
        printf("Data was only partly sent (%d/%d bytes).\n", sentd, len);
    } else {
        printf("Total bytes sent is %d.\n", sentd);
    }

    return sentd;
}

char* readfromfile(char* file) {
    FILE* f = fopen(file, "r");
    if (f == NULL) {
        printf("Error in opening file\n");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 

    char* buffer = (char*)malloc(fsize + 1);
    if (buffer == NULL) {
        printf("Error in allocating memory\n");
        return NULL;
    }

    fread(buffer, 1, fsize, f);
    fclose(f);
    buffer[fsize] = '\0';
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <IP> <port> <algorithm>\n", argv[0]);
        return 1;
    }
    int file_size;
    do{
        printf("Enter the size of the file to be sent (minimum size is 2MB): ");
        scanf("%d", &file_size);
    }while(file_size < MIN_SIZE);

    char* rand_file = util_generate_random_data(file_size);

    char* ip = argv[1];
    int port = atoi(argv[2]);
    char* algo = argv[3];

    struct sockaddr_in serverAddress;
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket < 0) {
        printf("Error in creating socket\n");
        return 1;
    }
    if (strcmp(algo, "reno") == 0 || strcmp(algo, "cubic") == 0) {
        if (setsockopt(network_socket, IPPROTO_TCP, TCP_CONGESTION, algo, strlen(algo) + 1) < 0) {
            printf("Error setting congestion control algorithm\n");
            return 1;
        }
    } else {
        printf("Invalid algorithm\n");
        return 1;
    }

    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip);


    if (connect(network_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("Error in connecting to server\n");
        return 1;
    }


    char again;
    do {
    
        sendData(network_socket, rand_file, file_size);
        send(network_socket, "\exit", strlen("\exit") + 1, 0);
        printf("Do you want to send the file again? type y for yes, any other character for no\n");
        scanf(" %c", &again);
    } while (again == 'y' || again == 'Y');

    close(network_socket);
    free(rand_file);


    return 0;
}