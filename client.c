#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sock;

// Receive messages
void *receive_msg(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        printf("%s\n", buffer);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];
    char name[50];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Enter your name: ");
    scanf("%s", name);
    send(sock, name, strlen(name), 0);

    pthread_t tid;
    pthread_create(&tid, NULL, receive_msg, NULL);

    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;