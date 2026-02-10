/* 
 * Roll Number: MT25074
 * Part A_Baseline_Client.c
 * Basic TCP Client - Connection Establishment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#define SERVER_IP "10.0.0.1"
#define SERVER_PORT 8080
#define NUM_FIELDS 8
#define DURATION_SECONDS 10  // Fixed duration - CLIENT controls this

struct message {
    char *fields[NUM_FIELDS];
};

struct message* create_message(size_t field_size) {
    struct message *msg = malloc(sizeof(struct message)); //memory for struct message which holds pointers to 8 string data
    if (!msg) return NULL;
    
    for (int i = 0; i < NUM_FIELDS; i++) {
        msg->fields[i] = malloc(field_size); //allocating heap memory and storing address in string pointer field
        if (!msg->fields[i]) {
            for (int j = 0; j < i; j++) free(msg->fields[j]);
            free(msg);
            return NULL;
        }
        memset(msg->fields[i], 'C' + i, field_size);  // Client pattern
   }
    return msg;
}

void free_message(struct message *msg) {
    if (!msg) return;
    for (int i = 0; i < NUM_FIELDS; i++) free(msg->fields[i]);
    free(msg);
}

ssize_t recv_all(int sockfd, void *buffer, size_t len) {
    char *buf = buffer;
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(sockfd, buf + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return total;
        total += n;
    }
    return total;
}

ssize_t send_all(int sockfd, const void *buffer, size_t len) {
    const char *buf = buffer;
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sockfd, buf + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
            return -1;
        }
        total += n;
    }
    return total;
}

int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <field_size>\n", argv[0]);
        exit(1);
    }

    size_t field_size = atoi(argv[1]);
    int port = SERVER_PORT;
    int duration = DURATION_SECONDS;
    
    
    printf("Client: Server=%s:%d, field_size=%zu, duration=%ds\n", SERVER_IP, port, field_size, duration);
  

    int sock_fd;
    struct sockaddr_in server_addr; 

    /************************************************/

    //STEP 1: CREATE A CLIENT SOCKET
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully (fd: %d)\n", sock_fd);


    /************************************************/

    //STEP 2: PREPARE SERVER ADDRESS

    memset(&server_addr, 0, sizeof(server_addr)); //intialise struct to zeroes
    server_addr.sin_family = AF_INET; //set address family type as IP NET
    server_addr.sin_port = htons(port);

    //convert IP string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Inet_pton function failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }


    /************************************************/

    //STEP 3: CONNECT TO OUR SERVER BY BLOCKING THE PROGRAM UNTIL CONNECTION ESTABLISH/FAILS

    printf("Attempting to connect to %s:%d...\n", SERVER_IP, SERVER_PORT);
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect function failed at client side");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Connection Established Successfully !!
    printf("Connected to server successfully!\n");
    printf("  Server IP: %s\n", SERVER_IP);
    printf("  Seerver Poort: %d\n", SERVER_PORT);
    printf("  Client socket fd: %d\n", sock_fd);


    
    /************************************************/

    //STEP 5: SEND AND RECIEVE DATA

    struct message *send_msg = create_message(field_size);
    struct message *recv_msg = create_message(field_size);
    
    if (!send_msg || !recv_msg) {
        perror("Failed to allocate messages");
        exit(1);
    }
    
    // ==================== FIXED DURATION TRANSFER ====================
    // CLIENT controls the duration - sends continuously for fixed time
    
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;
    uint64_t messages_sent = 0;
    uint64_t total_bytes = 0;
    
    printf("Starting transfer for %d seconds...\n", duration);
    
    // Send continuously until time expires
    while (time(NULL) < end_time) {
        // Send request: all 8 fields
        for (int i = 0; i < NUM_FIELDS; i++) {
            if (send_all(sock_fd, send_msg->fields[i], field_size) != field_size) {
                perror("Send failed");
                
            }
        }
        
        // Receive response: all 8 fields (server's repeated transfer)
        for (int i = 0; i < NUM_FIELDS; i++) {
            if (recv_all(sock_fd, recv_msg->fields[i], field_size) != field_size) {
                printf("Server closed connection unexpectedly\n");
                
            }
        }
        
        messages_sent++;
        total_bytes += (NUM_FIELDS * field_size * 2);  // Request + response
    }
    
    // Time's up - client initiates close
    printf("Time limit reached (%d seconds)\n", DURATION_SECONDS);
    printf("Messages exchanged: %lu\n", messages_sent);
    //printf("Throughput: %.2f Mbps\n", (total_bytes * 8.0) / (DURATION_SECONDS * 1000000.0));
    

    free_message(send_msg);
    free_message(recv_msg);
    
    // CLIENT closes connection - this signals server to stop
    printf("Closing connection...\n");



    /************************************************/

    //STEP 6: CLEAN UP / CLOSE THE SOCKETS FILES / FREE UP BUFFERS ETC..
    close(sock_fd);
    printf("Client shutting down...\n");
    
    return 0;
    

}