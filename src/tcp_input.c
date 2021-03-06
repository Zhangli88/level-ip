#include "syshead.h"
#include "tcp.h"
#include "tcp_text.h"
#include "skbuff.h"
#include "sock.h"

static int tcp_verify_segment(struct tcp_sock *tsk, struct tcphdr *th, struct tcp_segment *seg)
{
    struct tcb *tcb = &tsk->tcb;

    return 0;
}

static inline int tcp_discard(struct tcp_sock *tsk, struct sk_buff *skb, struct tcphdr *th)
{
    free_skb(skb);
    return 0;
}

static int tcp_listen(struct tcp_sock *tsk, struct sk_buff *skb, struct tcphdr *th)
{
    return 0;
}

static int tcp_synsent(struct tcp_sock *tsk, struct sk_buff *skb, struct tcphdr *th)
{
    struct tcb *tcb = &tsk->tcb;
    if (th->ack) {
        if (th->ack_seq <= tcb->iss || th->ack_seq > tcb->snd_nxt) {
            if (th->rst) goto discard;

            goto reset_and_discard;
        }

        if (!(tcb->snd_una <= th->ack_seq && th->ack_seq <= tcb->snd_nxt))
            goto reset_and_discard;
    }

    if (th->rst) {
        goto reset_and_discard;
    }

    if (!th->syn) {
        goto discard;
    }

    tcb->rcv_nxt = th->seq + 1;
    tcb->irs = th->seq;
    if (th->ack) {
        /* Any packets in RTO queue that are acknowledged here should be removed */
        tcb->snd_una = th->ack_seq;
    }

    if (tcb->snd_una > tcb->iss) {
        tsk->sk.state = TCP_ESTABLISHED;
        tcb->seq = tcb->snd_nxt;
        tcp_send_ack(&tsk->sk);
        wait_wakeup(&tsk->sk.sock->sleep);
    }
    
    return 0;
discard:
    return 0;
reset_and_discard:
    return 0;
}

static int tcp_drop(struct tcp_sock *tsk, struct sk_buff *skb)
{
    return 0;
}

static int tcp_closed(struct tcp_sock *tsk, struct sk_buff *skb, struct tcphdr *th)
{
    /*
      All data in the incoming segment is discarded.  An incoming
      segment containing a RST is discarded.  An incoming segment not
      containing a RST causes a RST to be sent in response.  The
      acknowledgment and sequence field values are selected to make the
      reset sequence acceptable to the TCP that sent the offending
      segment.

      If the ACK bit is off, sequence number zero is used,

        <SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK>

      If the ACK bit is on,

        <SEQ=SEG.ACK><CTL=RST>

      Return.
    */

    int rc = -1;

    if (th->rst) {
        tcp_discard(tsk, skb, th);
    }

    if (th->ack) {
 
    } else {
        
    
    }
    
    rc = tcp_send_reset(tsk);

    free_skb(skb);
    
    return rc;
}

/*
 * Follows RFC793 "Segment Arrives" section closely
 */ 
int tcp_input_state(struct sock *sk, struct sk_buff *skb, struct tcp_segment *seg)
{
    struct tcphdr *th = tcp_hdr(skb);
    struct tcp_sock *tsk = tcp_sk(sk);
    struct tcb *tcb = &tsk->tcb;

    switch (sk->state) {
    case TCP_CLOSE:
        return tcp_closed(tsk, skb, th);
        goto discard;
    case TCP_LISTEN:
        return tcp_listen(tsk, skb, th);
    case TCP_SYN_SENT:
        return tcp_synsent(tsk, skb, th);
    }

    /* "Otherwise" section in RFC793 */

    /* first check sequence number */
    if (tcp_verify_segment(tsk, th, seg) < 0) {
        return 0;
    }
    
    /* second check the RST bit */

    if (th->rst) {

    }
    
    /* third check security and precedence */
    // Not implemented

    /* fourth check the SYN bit */
    if (th->syn) {

    }
    
    /* fifth check the ACK field */
    if (!th->ack) {
        tcp_drop(tsk, skb);
    }

    /* ACK bit is on */

    switch (sk->state) {
    case TCP_SYN_RECEIVED:
    case TCP_ESTABLISHED:
        if (tcb->snd_una < seg->ack && seg->ack <= tcb->snd_nxt) {
            tcb->snd_una = seg->ack;
            /* TODO: Any segments on the retransmission queue which are thereby
               entirely acknowledged are removed. */
            
        }

        if (seg->ack < tcb->snd_una) {
            // Ignore
        }

        if (seg->ack > tcb->snd_nxt) {
            tcp_send_ack(&tsk->sk);
            tcp_drop(tsk, skb);
            return 0;
        }
    }
    
    /* sixth, check the URG bit */
    if (th->urg) {

    }
    
    /* seventh, process the segment txt */
    switch (sk->state) {
    case TCP_ESTABLISHED:
    case TCP_FIN_WAIT_1:
    case TCP_FIN_WAIT_2:
        /* deliver segment text to user RECEIVE buffers */
        tcp_data_queue(tsk, th, seg);
        
        break;
    case TCP_CLOSE_WAIT:
    case TCP_CLOSING:
    case TCP_LAST_ACK:
    case TCP_TIME_WAIT:
        /* This should not occur, since a FIN has been received from the
           remote side.  Ignore the segment text. */
        break;
    }


    

    /* eighth, check the FIN bit */
    
    return 0;
    
discard:
    return tcp_drop(tsk, skb);
}

int tcp_data_queue(struct tcp_sock *tsk, struct tcphdr *th, struct tcp_segment *seg)
{
    struct tcb *tcb = &tsk->tcb;
    
    /* if (seg->seq == tcb->rcv_nxt) { */
    /*     if (!tcb->rcv_wnd) { */
    /*         goto out; */
    /*     } */

    /* } */

    tcp_write_buf(tsk, th->data, seg->dlen);

    if (th->psh) tsk->flags |= TCP_PSH;
    
    return tsk->sk.ops->recv_notify(&tsk->sk);
}

int tcp_receive(struct tcp_sock *tsk, void *buf, int len)
{
    int rlen = 0;
    int curlen = 0;
    

    while (rlen < len) {
        curlen = tcp_read_buf(tsk->rcv_buf, buf + rlen, len - rlen);

        rlen += curlen;

        printf("reading tcp\n.");

        if (tsk->flags & TCP_PSH) {
            tsk->flags &= ~TCP_PSH;
            break;
        }
        
        wait_sleep(&tsk->sk.recv_wait);

        printf("woke up!\n");
    }

    
    
    return 0;
}
