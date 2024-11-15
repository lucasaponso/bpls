#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 12345
#define BUFFER_SIZE 128  // Adjust based on the hex data size
#define SEND_INTERVAL 20  // Interval in seconds to send data (e.g., every 5 seconds)

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    uint8_t hex_data[] = {
        0x7c, 0x56, 0x00, 0x00, 0x42, 0x08, 0x01, 0x30, 0x32, 0x34, 0x37, 0x00, 0x32, 0x31, 0x38, 0x39,
        0x33, 0x43, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x2e, 0x30, 0x30, 0x00, 0x31, 0x31, 0x2f, 0x30, 
        0x35, 0x2f, 0x32, 0x30, 0x32, 0x34, 0x20, 0x30, 0x35, 0x3a, 0x31, 0x37, 0x3a, 0x33, 0x36, 0x20, 
        0x50, 0x4d, 0x00, 0x4d, 0x69, 0x6e, 0x65, 0x72, 0x61, 0x69, 0x00, 0x4c, 0x41, 0x56, 0x4f, 0x49, 
        0x45, 0x2c, 0x20, 0x44, 0x41, 0x4e, 0x49, 0x45, 0x4c, 0x00, 0x30, 0x2e, 0x30, 0x30, 0x00, 0x30, 
        0x2e, 0x30, 0x30, 0x00, 0x30, 0x2e, 0x30, 0x30, 0x00, 0x87, 0xbe
        }; //91 bytes
    // uint8_t hex_data[] = {
    //   0x7c, 0x0c, 0x00, 0x00, 0x01, 0x08, 0x02, 0x54, 0x31, 0x35, 0x31, 0x00, 0x31, 0x34, 0x35, 0xb7, 0x4f
    // };

    int data_size = sizeof(hex_data);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    server_addr.sin_port = htons(PORT);       // Convert port to network byte order

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Infinite loop to accept clients and send the hex data periodically
    client_len = sizeof(client_addr);
    while (1) {
        // Accept a client connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Client connection failed");
            continue;  // If accept fails, continue listening for other clients
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Send the hex data every SEND_INTERVAL seconds to the client
        while (1) {
            // Send the hex data to the client
            int bytes_sent = 0;
            if ((bytes_sent = send(client_fd, hex_data, data_size, 0)) < 0) {
                perror("Send failed");
                break;  // If send fails, break out of the inner loop
            } else {
                printf("Hex data sent to client.\n");
                printf("Bytes Sent: %d", bytes_sent);
            }
            ssize_t bytes_received;
            uint8_t buffer[BUFFER_SIZE];

            bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                perror("Receive failed");
                break;
            }
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
                break;
            }

            // Print the received data in hexadecimal format
            printf("Received data:\n");
            for (int i = 0; i < bytes_received; i++) {
                printf("0x%02X ", buffer[i]);
            }
            printf("\n");

            // Wait for the next interval before sending again
            sleep(SEND_INTERVAL);
        }

        // Close the client socket after sending data periodically
        close(client_fd);
        printf("Connection closed for client.\n");
    }

    // Server will never reach this point, but it's good practice to close the server socket
    close(server_fd);
    printf("Server socket closed.\n");

    return 0;
}
