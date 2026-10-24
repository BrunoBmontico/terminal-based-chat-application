#include <stdio.h>
#include <string.h>
#include <log.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


static int init_client(void) 
{   
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1) 
    {
        LOG(ERROR, strerror(errno));
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG(ERROR, strerror(errno));
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        LOG(ERROR, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

static void *recv_loop(void *arg) // entrada/saida void para valores genéricos
{
    int client_fd = *(int *)arg; // valor guardado no endereço
    char buffer[256];

    while (1)
    {
        int r = recv(client_fd, buffer, sizeof(buffer) -1, 0);
        if (r <= 0)
        {
            LOG(INFO, "Disconected from the server");
            break;
        }
        buffer[r] = '\0';
        LOG(INFO, buffer);
    }
    return NULL;
}

static int send_loop(int client_fd) 
{
    char message[256];
    int status = 0;
    while (1)
    {
        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            LOG(ERROR, "message is null");
            status = -1;
            break;
        }
        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "exit") == 0)
        {
            break;
        }

        if (send(client_fd, message, strlen(message), 0) < 0)
        {
            LOG(ERROR, strerror(errno));
            status = -1;
            break;
        }
    }
    close(client_fd);
    return status;
}

int main()
{
    int client_fd = init_client();
    if (client_fd < 0) 
    {
        LOG(ERROR, strerror(errno));
        return -1;
    }
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_loop, &client_fd);
    send_loop(client_fd);
    pthread_join(recv_thread, NULL);
    return 0;
}