#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Client structure
typedef struct {
    int socket;
    char name[50];
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t mutex;

// Broadcast message
void broadcast(char *message, int sender_sock) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_sock) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}    
// Private message (@username message)
void private_message(char *message, int sender_sock) {
    char target[50], msg[BUFFER_SIZE];
    sscanf(message, "@%s %[^\n]", target, msg);

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].name, target) == 0) {
            send(clients[i].socket, msg, strlen(msg), 0);
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
}
// Handle each client
void *handle_client(void *arg) {
    int sock = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char name[50];

    recv(sock, name, sizeof(name), 0);

    pthread_mutex_lock(&mutex);
    strcpy(clients[client_count].name, name);
    clients[client_count].socket = sock;
    client_count++;
    pthread_mutex_unlock(&mutex);

    printf("%s connected\n", name);

    while (1) {
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';

        if (buffer[0] == '@') {
            private_message(buffer, sock);
        } else {
            broadcast(buffer, sock);
        }
    }
  // Remove client
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == sock) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    close(sock);
    printf("%s disconnected\n", name);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    pthread_mutex_init(&mutex, NULL);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    listen(server_sock, MAX_CLIENTS);

    printf("Server started on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client, &len);

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &client_sock);
    }  
close(server_sock);
    return 0;
}