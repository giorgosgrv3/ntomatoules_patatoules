#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>



#define SERVER_PORT "14444"
#define BUFFER_SIZE 65536

/*  stdio.h, stdlib.h, string.h, unistd.h: C standard I/O and memory functions.      
    sys/types.h, sys/socket.h, netinet/in.h, arpa/inet.h: For sockets and networking.
    netdb.h: For getaddrinfo() and related DNS utilities.
    errno.h: To handle and print system call errors.*/


void *get_in_addr(struct sockaddr *sa) { //helper function when getaddrinfo() gives us an address structure
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

/*
struct sockaddr {
    unsigned short sa_family;   // Address family (AF_INET for IPv4)
    char sa_data[14];           // Protocol-specific address data
};

struct sockaddr_in {
    short int          sin_family;  // AF_INET
    unsigned short int sin_port;    // Port number (network byte order)
    struct in_addr     sin_addr;    // IP address
    char               sin_zero[8]; // Padding (unused)

    we can cast between the two in system calls:
    connect(sockfd, (struct sockaddr *)&their_addr, sizeof their_addr);

};
*/

int main(int argc, char *argv[]) {
    printf("BOOT OK\n");
    int sockfd;
    struct addrinfo hints, *servinfo, *p; // hints -> to specify what kind of addresses we want.
    // *servinfo, *p -> to store the results and iterate through them.
    int rv;
    char s[INET_ADDRSTRLEN];  // To hold the printable IP

    if (argc != 2) {
        fprintf(stderr, "usage: %s <server-ip-address>\n", argv[0]);
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;        // IPv4 only
    hints.ai_socktype = SOCK_STREAM;  // TCP

    printf("[DEBUG] Calling getaddrinfo() for host: %s, port: %s\n", argv[1], SERVER_PORT);

    //int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
    if ((rv = getaddrinfo(argv[1], "14444", &hints, &servinfo)) != 0) {
        fprintf(stdout, "[ERROR] getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    printf("[DEBUG] Starting connection attempt...\n");
    for (p = servinfo; p != NULL; p = p->ai_next) {
        printf("[DEBUG] Trying socket creation...\n");
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("client: socket");
        continue;
    }
    printf("[DEBUG] Socket created, trying connect...\n");


    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("client: connect");
        continue;
    }
    printf("[DEBUG] Connect succeeded.\n");


    break; // success
}

    if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

     // Convert the IP to a string and print it
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // done with this structure

    ////// Sending data /////

    char buffer[BUFFER_SIZE]; // We create the buffer
    memset(buffer, 'A', sizeof(buffer));  // Fill buffer with dummy data to send, the character 'A'

    struct timeval start_time, current_time;
    gettimeofday(&start_time, NULL);  // Capture start time

    long total_bytes_sent = 0;
    long interval_bytes_sent = 0;

    int duration_seconds = 30;
    int interval_seconds = 2;

    while (1) {
    // Send the buffer
    ssize_t bytes_sent = send(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_sent < 0) {
        perror("client: send");
        break;
    }

    total_bytes_sent += bytes_sent;
    interval_bytes_sent += bytes_sent;

    // Check current time
    gettimeofday(&current_time, NULL);
    double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                     (current_time.tv_usec - start_time.tv_usec) / 1e6;

    // Print throughput every 2 seconds
    if (((int)elapsed % interval_seconds == 0) && (elapsed >= interval_seconds)) {
        double interval_mbps = (interval_bytes_sent * 8.0) / 1e6 / interval_seconds;
        printf("Time %.0f sec: %.2f Mbps\n", elapsed, interval_mbps);
        interval_bytes_sent = 0;

        // Sleep a bit to avoid duplicate prints in same second
        struct timespec ts = {0, 100 * 1000 * 1000}; // 0.1 seconds
        nanosleep(&ts, NULL);

    }

    // Exit after 30 seconds
    if (elapsed >= duration_seconds) {
        break;
    }
}

    // After the loop, printing final stats
    double total_mbps = (total_bytes_sent * 8.0) / 1e6 / duration_seconds;
    printf("Total throughput: %.2f Mbps\n", total_mbps);

    close(sockfd);

    return 0;
}