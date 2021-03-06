#ifndef IPV4_H
#define IPV4_H
#include "syshead.h"
#include "skbuff.h"
#include "sock.h"

#define IPV4 0x04
#define IP_TCP 0x06
#define ICMPV4 0x01

#define IP_HDR_LEN sizeof(struct iphdr)
#define ip_len(ip) (ip->len - (ip->ihl * 4))

struct iphdr {
    uint8_t ihl : 4; /* TODO: Support Big Endian hosts */
    uint8_t version : 4;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t flags : 3;
    uint16_t frag_offset : 13;
    uint8_t ttl;
    uint8_t proto;
    uint16_t csum;
    uint32_t saddr;
    uint32_t daddr;
    uint8_t data[];
} __attribute__((packed));

static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
    return (struct iphdr *)(skb->head + ETH_HDR_LEN);
}

int ip_rcv(struct sk_buff *skb);
int ip_output(struct sock *sk, struct sk_buff *skb);
    
#endif
