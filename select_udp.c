#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MESSAGE_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return 1;
    }

    int socket_fd;
    struct sockaddr_in server_addr, client_addr;
    char message[MAX_MESSAGE_LEN];

    // Create socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("Error creating socket");
        return 1;
    }
    int first_port = atoi(argv[2]);
    int sec_port = atoi(argv[3]);
    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(first_port);
    if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
        perror("Invalid IP address");
        return 1;
    }

    // Bind socket to client address
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(sec_port);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(socket_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
        perror("Error binding socket");
        return 1;
    }

    printf("Connected. Start typing your messages.\n");

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);  // Add stdin to the set
        FD_SET(socket_fd, &read_fds);  // Add socket to the set

        // Monitor file descriptors for activity
        if (select(socket_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Error in select");
            return 1;
        }

        if (FD_ISSET(0, &read_fds)) {  // Data available on stdin
            // Read input from stdin
            fgets(message, sizeof(message), stdin);
            // Send message to server
            sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }

        if (FD_ISSET(socket_fd, &read_fds)) {  // Data available on socket
            // Receive message from server
            ssize_t recv_len = recvfrom(socket_fd, message, sizeof(message) - 1, 0, NULL, NULL);
            if (recv_len == -1) {
                perror("Error receiving message");
                return 1;
            }
            message[recv_len] = '\0';
            printf("Received: %s", message);
        }
    }

    close(socket_fd);
    return 0;
}
