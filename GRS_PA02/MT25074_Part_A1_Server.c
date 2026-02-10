/* 
 * Roll Number: MT25074
 * Part A_Baseline_Server.c
 * Basic TCP Server - Connection Establishment
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
#include <pthread.h>


#define SERVER_IP "10.0.0.1"
#define SERVER_PORT 8080
#define BACKLOG 5
#define NUM_FIELDS 8

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
        memset(msg->fields[i], 'S' + i, field_size);  // Server pattern
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
        if (n == 0) return total;  // Connection closed
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
        }
        total += n;
    }
    return total;
}



/************************************************/

//THREAD HANDLING FOR THE CLIENTS


struct thread_args{
    int conn_fd;
    size_t field_size;
};

void* client_thread(void* arg) {

    struct thread_args *args = (struct thread_args*)arg;

    // Server's response message (created once, sent repeatedly)
    struct message *response_msg = create_message(args->field_size);
    // Buffer to receive client request
    struct message *request_buffer = create_message(args->field_size);
    
    if (!response_msg || !request_buffer) {
        close(args->conn_fd);
        free(args);
        return NULL;
    }

    uint64_t msg_count = 0; 
    
    // ==================== REPEATED TRANSFERS ====================
    // Server transfers repeatedly until client closes connection
    // NO TIME LIMIT on server side - just responds to each client message
    
    while (1) {
        
        // STEP 1: Receive all 8 fields from client (request)
        for (int i = 0; i < NUM_FIELDS; i++) {
            ssize_t n = recv_all(args->conn_fd, request_buffer->fields[i], args->field_size);
            
            if (n == 0 && i == 0) {
                // Client closed connection cleanly
                printf("Server: Client closed connection. Thread handled - Total messages: %lu\n", msg_count);
                goto cleanup;
            }
    
            if (n != args->field_size) {
                printf("Server: Receive error or partial (%zd/%zu)\n", n, args->field_size);
                goto cleanup;
            }
        }
        
        // STEP 2: Send all 8 fields back to client (response)
        // This is the "transfer" - server responds with fixed-size message
        for (int i = 0; i < NUM_FIELDS; i++) {
            ssize_t n = send_all(args->conn_fd, response_msg->fields[i], args->field_size);
            if (n != args->field_size) {
                printf("Server: Send failed\n");
                goto cleanup;
            }
        }
        
        msg_count++;
        

    }
    
cleanup:
    free_message(response_msg);
    free_message(request_buffer);
    close(args->conn_fd);
    free(args);
    return NULL;
}





int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <field_size> <num_threads>\n", argv[0]);        exit(1);
        fprintf(stderr, "Got %d arguments\n", argc-1);
        for (int i = 1; i < argc; i++) {
            fprintf(stderr, "  arg[%d]: %s\n", i, argv[i]);
        }
        
        exit(1);
    }
    
    size_t field_size = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    printf("Server: field_size=%zu, accepting %d clients\n", field_size, num_threads);



    int listen_fd, conn_fd; //Listening socket file, Connection Established Socket file
    struct sockaddr_in server_addr, client_addr; //structs holding server socket , client socket addresses
    socklen_t addr_len  = sizeof(client_addr); //holds the size of (SockAddress Struct) which is the maximum client address size possible


    /************************************************/

    //STEP 1: CREATE A LISTENING SOCKET

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(listen_fd < 0){
        perror("server socket creation failed "); //creation failed
    }
    printf("Socket created successfully (fd: %d)\n", listen_fd); //Success Heres the socket fd 



    /************************************************/

    //STEP 2: BIND THE LISTENING SOCKET TO ADDRESS AND PORT

    //Create the IP address structure
    memset(&server_addr, 0, sizeof(server_addr)); //initialise the address structure to zeroes for safety
    server_addr.sin_family = AF_INET; //address family domain is IP internet
    server_addr.sin_port = htons(SERVER_PORT); // host byte order to network byte order

    //Convert IP address string to Binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed to convert server IP structure to Binary form");
        close(listen_fd);   //close if pton function fails and exit the program
        exit(EXIT_FAILURE);
    }

    //Bind our listening socket to this binary form address   
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("BInd failed to listening socket");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Socket bound to %s:%d\n", SERVER_IP, SERVER_PORT);



    /************************************************/

    //STEP 3: LISTEN FOR INCOMING CONNECTIONS FROM OUR LISTENING SOCKET

    if (listen(listen_fd, BACKLOG)) {   //backlog is a limit number for pending connection requests
        perror("Listening function failed for the server listening socket");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server Listening on %s:%d\n", SERVER_IP, SERVER_PORT);



    /************************************************/

    //STEP 4: ACCEPT AN INCOMING CONNECTION IF AVAIALABLE BY BLOCKING EVERYTHING AND TCP CONNECT

    printf("Waiting for client connection...\n"); //Starting the accepting and handhaking connection process


    pthread_t tids[num_threads];

    //ACCEPT N CLIENTS
    for (int i = 0; i < num_threads; i++) {

        conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (conn_fd < 0) {
            perror("Accept failed");
            continue; //Try next client on the queue
            // close(listen_fd);
            // exit(EXIT_FAILURE);
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
        printf("Client %d/%d from %s:%d\n", i+1, num_threads, ip, ntohs(client_addr.sin_port));

        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->conn_fd = conn_fd;
        args->field_size = field_size;
    
        pthread_create(&tids[i], NULL, client_thread, args);

    }

    close(listen_fd);
    printf("All clients connected, waiting...\n");

    //waiting fro all threads to finish execution...
    for (int i = 0; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
    }

    printf("All done. Server exiting.\n");
    return 0;

    
}