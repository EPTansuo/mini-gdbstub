#include "conn.h"
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

static bool socket_readable(int socket_fd, int timeout)
{
    struct pollfd pfd = (struct pollfd){
        .fd = socket_fd,
        .events = POLLIN,
    };

    return (poll(&pfd, 1, timeout) > 0) && (pfd.revents & POLLIN);
}

bool conn_init(conn_t *conn, char *addr_str, int port)
{
    conn->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn->listen_fd < 0)
        return false;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(addr_str);
    addr.sin_port = htons(port);
    if (bind(conn->listen_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        goto fail;

    if (listen(conn->listen_fd, 1) < 0)
        goto fail;

    conn->socket_fd = accept(conn->listen_fd, NULL, NULL);
    if (conn->socket_fd < 0)
        goto fail;

    return true;

fail:
    close(conn->listen_fd);
    return false;
}

void conn_recv(conn_t *conn)
{
    static int i = 0;

    /* TODO: read a full GDB packet and return to handle it */
    while (socket_readable(conn->socket_fd, -1)) {
        char c;
        int nread = read(conn->socket_fd, &c, 1);
        if (nread == -1)
            break;

        if (nread > 0)
            printf("%d: c = %c\n", i++, c);
    }
}

void conn_close(conn_t *conn)
{
    close(conn->socket_fd);
    close(conn->listen_fd);
}
