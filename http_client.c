#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define HTTP_PORT "80"

int create_socket_and_connect(const char *hostname) {
    struct addrinfo hints, *res, *p;
    int sockfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname, HTTP_PORT, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "DNS resolution failed: %s\n", gai_strerror(status));
        return -1;
    }

    // Try all addresses until successful connection
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket creation failed");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connection failed");
            continue;
        }

        // Successfully connected
        break;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "Failed to connect to %s\n", hostname);
        return -1;
    }

    printf("Connected to %s\n", hostname);
    return sockfd;
}

void send_http_get_request(int sockfd, const char *hostname, const char *path) {
    char request[BUFFER_SIZE];
    
    // Validate input lengths
    size_t required_len = snprintf(NULL, 0, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);
    if (required_len >= BUFFER_SIZE) {
        fprintf(stderr, "Request too large (max %d bytes)\n", BUFFER_SIZE);
        return;
    }

    // Safe string formatting
    int len = snprintf(request, sizeof(request),
                      "GET %s HTTP/1.1\r\n"
                      "Host: %s\r\n"
                      "Connection: close\r\n\r\n",
                      path, hostname);
    
    if (len < 0) {
        perror("request formatting failed");
        return;
    }

    ssize_t bytes_sent = send(sockfd, request, len, 0);
    if (bytes_sent == -1) {
        perror("send failed");
    } else {
        printf("HTTP GET request sent to %s%s\n", hostname, path);
    }
}

void receive_http_response(int sockfd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    printf("HTTP Response:\n");
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer)-1, 0)) {
        if (bytes_received == -1) {
            perror("recv failed");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }
}

void interact_with_server(const char *hostname, const char *path) {
    printf("Interacting with server %s...\n", hostname);

    int sockfd = create_socket_and_connect(hostname);
    if (sockfd < 0) return;

    send_http_get_request(sockfd, hostname, path);
    receive_http_response(sockfd);
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <hostname> [path]\n", argv[0]);
        return 1;
    }

    const char *hostname = argv[1];
    const char *path = (argc >= 3) ? argv[2] : "/";

    // Validate input lengths
    if (strlen(hostname) > 253 || strlen(path) > 2048) {
        fprintf(stderr, "Invalid input: hostname too long or path too large\n");
        return 1;
    }

    interact_with_server(hostname, path);
    return 0;
}