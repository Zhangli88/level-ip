#include "syshead.h"
#include "inet.h"
#include "socket.h"
#include "sock.h"
#include "tcp.h"
#include "wait.h"

extern struct net_ops tcp_ops;

static int inet_stream_connect(struct socket *sock, const struct sockaddr *addr,
                               int addr_len, int flags);

static int INET_OPS = 1;

struct net_family inet = {
    .create = inet_create,
};

static struct sock_ops inet_stream_ops = {
    .connect = &inet_stream_connect,
    .write = &inet_write,
    .read = &inet_read,
    .free = &inet_free,
};

static struct sock_type inet_ops[] = {
    {
        .sock_ops = &inet_stream_ops,
        .net_ops = &tcp_ops,
        .type = SOCK_STREAM,
        .protocol = IPPROTO_TCP,
    }
};

int inet_create(struct socket *sock, int protocol)
{
    struct sock *sk;
    struct sock_type *skt = NULL;

    for (int i = 0; i < INET_OPS; i++) {
        if (inet_ops[i].type == sock->type) {
            skt = &inet_ops[i];           
        }
    }

    if (!skt) {
        perror("Could not find socktype for socket\n");
        return 1;
    }

    sock->ops = skt->sock_ops;

    sk = sk_alloc(skt->net_ops, protocol);
    sk->protocol = protocol;
    
    sock_init_data(sock, sk);
    
    return 0;
}

int inet_socket(struct socket *sock, int protocol)
{
    return 0;
}

int inet_connect(struct socket *sock, struct sockaddr *addr,
                 int addr_len, int flags)
{
    return 0;
}

static int inet_stream_connect(struct socket *sock, const struct sockaddr *addr,
                        int addr_len, int flags)
{
    struct sock *sk = sock->sk;
    int err;
    
    if (addr_len < sizeof(addr->sa_family)) {
        return -EINVAL;
    }

    if (addr->sa_family == AF_UNSPEC) {
        err = sk->ops->disconnect(sk, flags);
        sock->state = err ? SS_DISCONNECTING : SS_UNCONNECTED;
        goto out;
    }

    switch (sock->state) {
    default:
        err = -EINVAL;
        goto out;
    case SS_CONNECTED:
        err = -EISCONN;
        goto out;
    case SS_CONNECTING:
        err = -EALREADY;
        goto out;
    case SS_UNCONNECTED:
        err = -EISCONN;
        if (sk->state != TCP_CLOSE) {
            goto out;
        }

        err = sk->ops->connect(sk, addr, addr_len, flags);
        wait_sleep(&sock->sleep);

        if (err < 0) {
            goto out;
        }

        sock->state = SS_CONNECTING;

        err = -EINPROGRESS;
        break;
    }
    
    return 0;

out:
    return err;
}

int inet_write(struct socket *sock, const void *buf, int len)
{
    struct sock *sk = sock->sk;

    return sk->ops->write(sk, buf, len);
}

int inet_read(struct socket *sock, void *buf, int len)
{
    struct sock *sk = sock->sk;

    return sk->ops->read(sk, buf, len);
}

struct sock *inet_lookup(struct sk_buff *skb, uint16_t sport, uint16_t dport)
{
    return socket_lookup(sport, dport)->sk;
}

int inet_free(struct socket *sock)
{
    struct sock *sk = sock->sk;

    return sk->ops->abort(sk);
}
