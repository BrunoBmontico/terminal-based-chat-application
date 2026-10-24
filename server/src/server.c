#include <stdio.h>
#include <string.h>
#include <log.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_EVENTS 64
#define MAX_CLIENTS 64

static int clients_fds[MAX_CLIENTS];
static int client_count = 0;

static int init_server(void) 
{
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG(ERROR, strerror(errno));
        return -1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG(ERROR, strerror(errno));
        close(fd);
        return -1;
    }

    if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        LOG(ERROR, strerror(errno));
        close(fd);
        return -1;
    }

    if (listen(fd, 10))
    {
        LOG(ERROR, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

static void broadcast(int sender_fd, const char *message, size_t lenght)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients_fds[i] != sender_fd)
        {
            send(clients_fds[i], message, lenght, 0);
        }
    }
}

static void remove_client(int fd)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients_fds[i] == fd)
        {
            clients_fds[i] = clients_fds[client_count - 1];
            client_count--;
            break;
        }
    }
}

static void handle_client(int ready_fd, struct epoll_event* buffer_ev, int socket_fd, int epfd) 
{   
    for (int i = 0; i < ready_fd; i++)
    {   
        int fd = buffer_ev[i].data.fd;

        if (fd == socket_fd)
        {
            int connection_fd = accept(socket_fd, 0, 0);
            if (connection_fd < 0)
            {
                if (errno == EINTR || errno == ECONNABORTED) // ERROS NAO BLOQUEANTES
                {
                    continue;
                }
                LOG(ERROR, strerror(errno));
                continue;
            }

            struct epoll_event client_ev = {0};
            client_ev.events = EPOLLIN;
            client_ev.data.fd = connection_fd;
            epoll_ctl(epfd, EPOLL_CTL_ADD, connection_fd, &client_ev);

            clients_fds[client_count++] = connection_fd;
            LOG(INFO, "Client connected");

        } else {
            char buffer[256];
            int r = recv(fd, buffer, sizeof(buffer) - 1, 0);

            if (r < 0) {
                LOG(ERROR, strerror(errno));
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd); remove_client(fd); } else if (r == 0) { LOG(INFO, "Client disconnected");
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                remove_client(fd);
            } else {
                buffer[r] = '\0';
                broadcast(fd, buffer, r);
            }
        }
    }
}

static int epoll_loop(int socket_fd)
{
    int epfd = epoll_create1(0);
    if (epfd < 0)
    {
        LOG(ERROR, strerror(errno));
        return -1;
    }

    struct epoll_event listen_ev = {0};
    listen_ev.events = EPOLLIN;
    listen_ev.data.fd = socket_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, &listen_ev);

    struct epoll_event buffer_ev[MAX_EVENTS];

    while (1)
    {
        int ready_fd = epoll_wait(epfd, buffer_ev, MAX_EVENTS, -1);
        if (ready_fd < 0)
        {
            LOG(ERROR, strerror(errno));
            continue;
        }
        handle_client(ready_fd, buffer_ev, socket_fd, epfd);
    }

    close(epfd);
    return 0;
}

int main()
{   
    LOG(INFO, "Initializing Server...");
    int socket_fd = init_server();
    if (socket_fd < 0)
    {
        LOG(ERROR, strerror(errno));
        return -1;
    }
    LOG(INFO, "Server is up!");
    epoll_loop(socket_fd);

    return 0;
}
