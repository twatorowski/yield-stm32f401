/**
 * @file tcp_sock.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief 
 * @date 2024-07-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "assert.h"
#include "config.h"
#include "err.h"
#include "net/tcpip/ip_addr.h"
#include "net/tcpip/tcp.h"
#include "net/tcpip/tcp_sock.h"
#include "util/elems.h"
#include "sys/queue.h"
#include "sys/sem.h"
#include "sys/time.h"
#include "sys/yield.h"
#include "util/elems.h"
#include "util/endian.h"
#include "util/minmax.h"
#include "util/string.h"

#define DEBUG
#include "debug.h"

/* sockets */
static tcpip_tcp_sock_t sockets[TCPIP_TCP_SOCK_NUM];
/* processing lock */
static sem_t lock;
/* local port number generator */
static tcpip_udp_port_t loc_port_num = 10000;


/* sends RST frame in reply to provided frame */
static err_t TCPIPTcpSock_Reject(tcpip_frame_t *frame)
{
    /* error code */
    err_t ec = EOK; tcpip_frame_t response;
    /* header pointers */
    tcpip_ip_frame_t *ip = frame->ip;
    tcpip_tcp_frame_t *tcp = frame->tcp;

    /* extract flags */
    tcpip_tcp_flags_t flags = TCPIPTcpFrame_GetFlags(tcp);
    /* never respond with rst to rst */
    if (flags & TCPIP_TCP_FLAGS_RST)
        return ec;
    
    /* allocate space for frame to be sent */
    if ((ec = TCPIPTcp_Alloc(&response)) != EOK)
        return ec;
    
    /* ip & ports */
    tcpip_ip_addr_t ip_addr = TCPIPIpFrame_GetSrcAddr(ip);
    tcpip_tcp_port_t src_port = TCPIPTcpFrame_GetSrcPort(tcp);
    tcpip_tcp_port_t dst_port = TCPIPTcpFrame_GetDstPort(tcp);

    /* sequencing numbers */
    uint32_t seq = TCPIPTcpFrame_GetSeq(tcp);
    uint32_t ack = TCPIPTcpFrame_GetAck(tcp);

    /* no data is contained */
    response.size = 0;
    /* sequencing depends on whether the ack is set */
    if (flags & TCPIP_TCP_FLAGS_ACK) {
        flags = TCPIP_TCP_FLAGS_RST;
        /* take the sequence number from the ack */
        seq = ack; ack = 0;
     /* no ack -> reset shall have the sequence number equal to zero and ack 
      * shall be generated based on flags and seq number received */
    } else {
        /* handle special flags */
        ack = flags & TCPIP_TCP_FLAGS_SYN ? seq + 1 : seq;
        seq = 0;
        /* ack number is valid - we can set the ack flag! */
        flags = TCPIP_TCP_FLAGS_ACK | TCPIP_TCP_FLAGS_RST;
    }

    /* try to send the frame */
    return TCPIPTcp_Send(&response, ip_addr, dst_port, src_port, 
        seq, ack, 0, flags);
}

/* process incoming frames */
static err_t TCPIPTcpSock_ProcessIncoming(tcpip_frame_t *frame, 
    tcpip_tcp_sock_t *sock)
{
    /* packet header */
    tcpip_tcp_frame_t *tcp = frame->tcp;

    /* socket is closed - it does not accept any traffic */
    if (sock->state == TCPIP_TCP_SOCK_STATE_FREE ||
        sock->state == TCPIP_TCP_SOCK_STATE_CLOSED)  {
        goto error;
    }
    
    /* extract port numbers */
    tcpip_tcp_port_t src_port = TCPIPTcpFrame_GetSrcPort(tcp);
    tcpip_tcp_port_t dst_port = TCPIPTcpFrame_GetDstPort(tcp);
    /* destination port mismatch */
    if (sock->loc_port != dst_port) {
        goto error;
    }
    
    /* get the source ip address */
    tcpip_ip_addr_t ip = TCPIPIpFrame_GetSrcAddr(frame->ip);
    /* at states above 'listen' the ip and  source port must match */
    if ((sock->state != TCPIP_TCP_SOCK_STATE_LISTEN) && 
         (sock->rem_port != src_port || 
         !TCPIPIpAddr_AddressMatch(ip, sock->addr))) {
        goto error;
    }

    /* get the sequence numbers as well */
    uint32_t seq = TCPIPTcpFrame_GetSeq(tcp);
    uint32_t ack = TCPIPTcpFrame_GetAck(tcp);
    /* and the window size */
    uint32_t win = TCPIPTcpFrame_GetWindow(tcp);
    /* extract flags */
    tcpip_tcp_flags_t rx_flags = TCPIPTcpFrame_GetFlags(tcp);

    /* reset segment with matching flags */
    if (rx_flags & TCPIP_TCP_FLAGS_RST) {
        /* mark socket as closed */
        sock->state = TCPIP_TCP_SOCK_STATE_CLOSED;
    }

    /* synchronization frame */
    if (rx_flags & TCPIP_TCP_FLAGS_SYN) {
        
        /* we are not listening and not trying to connect */
        if ((sock->state != TCPIP_TCP_SOCK_STATE_LISTEN) &&
            (sock->state != TCPIP_TCP_SOCK_STATE_CONNECT || 
                !(rx_flags & TCPIP_TCP_FLAGS_ACK)))
            goto error;

        /* handle the initialization of the counters */
        /* store the credentials */
        sock->addr = ip, sock->rem_port = src_port;
        /* along with the syn segment connection initiator sends it's
        * sequencing numbers */
        sock->rx_seq_recvd = seq + 1;
        sock->rx_seq_acked = seq;
        /* now we allow full window to be used */
        sock->rx_win = Queue_GetFree(sock->rxq);
        /* this is a perfect spot to initiate our sequence numbers */
        sock->tx_seq_start = time(0);
        sock->tx_win = win;
        /* reset retransmission stuff */
        sock->tx_retr_cnt = 0;
        sock->tx_retr_ts = 0;

        /* remote-site initiated connection  */
        if (sock->state == TCPIP_TCP_SOCK_STATE_LISTEN) {
            sock->state = TCPIP_TCP_SOCK_STATE_ESTABLISHING;
        /* local site initiated connection */
        } else {
            sock->state = TCPIP_TCP_SOCK_STATE_ESTABLISHED;
            /* update the link states */
            sock->loc_link = TCPIP_TCP_LINK_STATE_OPEN;
            sock->rem_link = TCPIP_TCP_LINK_STATE_OPEN;
        }

        /* store the timestamp of the frame received */
        sock->syn_fin_ts = time(0);
    }

    /* ack frame: remote site wants to ack out data */
    if (rx_flags & TCPIP_TCP_FLAGS_ACK) {
        /* special flags move the sequencing numbers by one w.r.t data */
        tcpip_frame_flags_t sf = TCPIP_TCP_FLAGS_FIN | TCPIP_TCP_FLAGS_SYN;

        /* completely wrong state */
        if (sock->state == TCPIP_TCP_SOCK_STATE_CLOSED ||
            sock->state == TCPIP_TCP_SOCK_STATE_LISTEN)
            goto error;
        
        /* check the ack counters */
        if (ack - sock->tx_seq_start > sock->tx_seq_end - sock->tx_seq_start)
            goto error;
        
        /* if this is a first ack after the out syn+ack then */
        if ((sock->state == TCPIP_TCP_SOCK_STATE_ESTABLISHING &&
             sock->tx_flags == (TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_ACK))) {
            /* update the socket state */
            sock->state = TCPIP_TCP_SOCK_STATE_ESTABLISHED;
            /* update the link states */
            sock->loc_link = TCPIP_TCP_LINK_STATE_OPEN;
            sock->rem_link = TCPIP_TCP_LINK_STATE_OPEN;
        }

        /* last ack from the remote site? */
        if (sock->loc_link == TCPIP_TCP_LINK_STATE_CLOSING) {
            sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSED;
            /* remote link was closed previously */
            if (sock->rem_link == TCPIP_TCP_LINK_STATE_CLOSED)
                sock->state = TCPIP_TCP_SOCK_STATE_CLOSED;
        }
        
        /* handle incoming data */
        /* only append data if it's in sequence */
        if (seq == sock->rx_seq_recvd) {
            /* special flags increment the sequence received */
            if (rx_flags & sf)
                sock->rx_seq_recvd += 1;
            /* store the incoming data */
            size_t b_stored = Queue_Put(sock->rxq, frame->ptr, frame->size);
            /* update the sequence number according to accepted data size */
            sock->rx_seq_recvd = sock->rx_seq_recvd + b_stored;
        /* remote site may have not received our last ack to what it sent */
        } else {
            /* this shall cause the re-acking */
            sock->rx_seq_acked = seq;
        }

        /* handle acknowledgement for outgoing data */
        /* let's see how many bytes are acknowledged */
        size_t b_acked = ack - sock->tx_seq_start;
        /* drop the data from the queue */
        Queue_Drop(sock->txq, b_acked);
        /* move the sequence numbers */
        sock->tx_seq_start += b_acked;
        /* reset the retransmission counter when all was acked */
        if (sock->tx_seq_start == sock->tx_seq_end)
            sock->tx_retr_cnt = 0;
    }

    /* remote party wants to finalize the connection */
    if (rx_flags & TCPIP_TCP_FLAGS_FIN) {
        /* indicate that the remote link is closing. it will be closed after 
         * we respond with ack to this fin */
        sock->state = TCPIP_TCP_SOCK_STATE_CLOSING;
        sock->rem_link = TCPIP_TCP_LINK_STATE_CLOSING;
        /* remote link closure causes local link starting to be closed as well 
         * */
        if (sock->loc_link == TCPIP_TCP_LINK_STATE_OPEN)
            sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSING;

        /* bump up the timestamp */
        sock->syn_fin_ts = time(0);
    }

    /* store the flags which can be used by the fsm */
    sock->rx_flags = rx_flags;
    /* return error code */
    return EOK;

    /* handle processing errors here */
    error: return EFATAL;
}

/* process outgoing traffic */
static void TCPIPTcpSock_ProcessOutgoing(tcpip_tcp_sock_t *sock)
{
    /* frame buffer */
    tcpip_frame_t frame;
    /* flags that we are about to transmit */
    tcpip_tcp_flags_t tx_flags = 0;

    /* pick the flags based on state */
    switch (sock->state) {
    /* states with no outgoing activity */
    case TCPIP_TCP_SOCK_STATE_FREE:
    case TCPIP_TCP_SOCK_STATE_CLOSED:
    case TCPIP_TCP_SOCK_STATE_LISTEN:
        goto end;

    /* we are trying to establish connection */
    case TCPIP_TCP_SOCK_STATE_CONNECT:
        tx_flags = TCPIP_TCP_FLAGS_SYN; break;
    /* we are trying to establish the connection */
    case TCPIP_TCP_SOCK_STATE_ESTABLISHING:
        tx_flags = TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_ACK; break;
    /* during connection we emmit the messages with ack */
    case TCPIP_TCP_SOCK_STATE_ESTABLISHED:
        tx_flags = TCPIP_TCP_FLAGS_ACK; break;
    /* connection is about to be closed */
    case TCPIP_TCP_SOCK_STATE_CLOSING: {
        /* continue emitting frames with ack to indicate that sequencing 
         * numbers are ok */
        tx_flags = TCPIP_TCP_FLAGS_ACK; 
        /* if we are in the closing state with our locak link then ensure that 
         * we emit the fin */
        if (sock->loc_link == TCPIP_TCP_LINK_STATE_CLOSING)
            tx_flags |= TCPIP_TCP_FLAGS_FIN;
    } break;
    }

    /* protect from syn floods and fin stalls */
    switch (sock->state) {
    /* these two states are the most sensitive ones */
    case TCPIP_TCP_SOCK_STATE_ESTABLISHING:
    case TCPIP_TCP_SOCK_STATE_CLOSING:
        if (dtime(time(0), sock->syn_fin_ts) > 2000) {
            sock->state = TCPIP_TCP_SOCK_STATE_CLOSED; goto end;
        }
    default: break;
    }

    /* no special flags, no need to ack anything, no data to sent, no 
     * changes in rx window */
    if ((tx_flags & (TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_FIN)) == 0 &&
        sock->rx_seq_recvd == sock->rx_seq_acked && 
        Queue_GetUsed(sock->txq) == 0 &&
        Queue_GetFree(sock->rxq) == sock->rx_win)
        goto end;
    
    /* check if enough time has passed since last retransmission */
    if ((dtime(time(0), sock->tx_retr_ts) < 300 * sock->tx_retr_cnt))
        goto end;

    /* allocate space for frame to be sent */
    if (TCPIPTcp_Alloc(&frame) != EOK)
        goto end;
    
    /* copy data to the frame payload section */ 
    frame.size = Queue_Peek(sock->txq, frame.ptr, frame.size);
    /* ensure that the data is pushed to the application on the remote site */
    if (frame.size > 0)
        tx_flags |= TCPIP_TCP_FLAGS_PSH;
    /* prepare the sequencing number denoting the end of the message */
    sock->tx_seq_end = sock->tx_seq_start + frame.size;
    /* are we sending any of the special flags? */
    if (tx_flags & (TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_FIN))
        sock->tx_seq_end += 1;
    
    /* build up the frame fields */
    uint32_t seq = sock->tx_seq_start;
    uint32_t ack = sock->rx_seq_recvd;
    uint16_t win = Queue_GetFree(sock->rxq);

    /* try to send the frame */
    if (TCPIPTcp_Send(&frame, sock->addr, sock->loc_port,
        sock->rem_port, seq, ack, win, tx_flags) < EOK)
        goto end;
    
    /* store tx flags that were emitted */
    sock->tx_flags = tx_flags;
    /* store the window size that was emmited */
    sock->rx_win = win;
    /* if the send was successful then we can move the rx ack numbers */
    sock->rx_seq_acked = sock->rx_seq_recvd;
    /* frame that was sent had payload besides the ack. if that's the case then 
     * prepare the variables for retransmission if it's needed */
    if (sock->tx_seq_end != sock->tx_seq_start)
        sock->tx_retr_cnt += 1, sock->tx_retr_ts = time(0);

    /* after we sent the ack to remote's fin we can close the remote link */
    if (sock->rem_link == TCPIP_TCP_LINK_STATE_CLOSING) {
        sock->rem_link = TCPIP_TCP_LINK_STATE_CLOSED;
        /* local link was already closed? it that's the case then remote link 
         * closing was the last frame to be received and we can close the 
         * entire socket */
        if (sock->loc_link == TCPIP_TCP_LINK_STATE_CLOSED)
            sock->state = TCPIP_TCP_SOCK_STATE_CLOSED;
    }
        
    /* end of processing */
    end: return;
}

/* socket fsm task task */
static void TCPIPTcpSock_Output(void *arg)
{
    /* tcp socket that is being processed */
    tcpip_tcp_sock_t *sock;
    /* processing for every socket */
    for (;; Yield()) {
        /* lock the socket access */
        Sem_Lock(&lock, 0);
        /* process all the sockets */
        for (sock = sockets; sock != sockets + elems(sockets); sock++)
            TCPIPTcpSock_ProcessOutgoing(sock);
        /* relase the socket access */
        Sem_Release(&lock);
    }
}

/* initialize tcp socket layer */
err_t TCPIPTcpSock_Init(void)
{
    /* create the task */
    Yield_Task(TCPIPTcpSock_Output, 0, 1024);
    /* report status */
    return EOK;
}

/* input routine to the socket layer */
err_t TCPIPTcpSock_Input(tcpip_frame_t *frame)
{
    /* socket pointer */
    tcpip_tcp_sock_t *sock; err_t ec = EFATAL;

    /* lock onto the sockets */
    Sem_Lock(&lock, 0);
    /* look for socket that this message may be directed to */
    for (sock = sockets; sock != sockets + elems(sockets); sock++)
        if ((ec = TCPIPTcpSock_ProcessIncoming(frame, sock)) == EOK)
            break;
    /* release the sockets */
    Sem_Release(&lock);

    /* wasn't able to process the frame, but do not answer. client will resend 
     * the frame at some later time */
    return ec == EOK ? EOK : EFATAL;
}

/* create a tcp socket */
tcpip_tcp_sock_t * TCPIPTcpSock_Create(size_t rx_size, size_t tx_size)
{
    /* socket pointer */
    tcpip_tcp_sock_t *sock;

    /* look for the free socket */
    for (sock = sockets; sock != sockets + elems(sockets); sock++)
        if (sock->state == TCPIP_TCP_SOCK_STATE_FREE)
            break;
    
    /* none found? */
    if (sock == sockets + elems(sockets))
        return 0;

    /* mark socket as closed so that others cannot allocate */
    sock->state = TCPIP_TCP_SOCK_STATE_CLOSED;
    /* create both queues for passing data to/from socket */
    sock->rxq = Queue_Create(1, rx_size);
    sock->txq = Queue_Create(1, tx_size);
    /* sanity check */
    assert(sock->rxq && sock->txq, "unable to allocate socket memory");
    /* return a pointer to sock structure */
    return sock;
}

/* start listening with socket on any given port */
err_t TCPIPTcpSock_Listen(tcpip_tcp_sock_t *sock, tcpip_tcp_port_t port)
{
    /* invalid socket provided */
    if (!(sock->state == TCPIP_TCP_SOCK_STATE_CLOSED) &&
        !(sock->state == TCPIP_TCP_SOCK_STATE_LISTEN))
        return EARGVAL;
    /* port number must be non zero */
    if (!port)
        return EARGVAL;
    
    /* reset control flags */
    sock->tx_flags = sock->rx_flags = 0;
    /* setup the port number and advance to listen state */
    sock->loc_port = port;
    sock->state = TCPIP_TCP_SOCK_STATE_LISTEN;
    /* close both sides of the link */
    sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSED;
    sock->rem_link = TCPIP_TCP_LINK_STATE_CLOSED;

    /* clear the queues */
    Queue_Drop(sock->rxq, Queue_GetUsed(sock->rxq));
    Queue_Drop(sock->txq, Queue_GetUsed(sock->txq));
    /* wait for someone to establish connection */
    for (;; Yield()) {
        /* connection is established */
        if (sock->state == TCPIP_TCP_SOCK_STATE_ESTABLISHED)
            break;
        /* connection was closed in the middle of establishment */
        if (sock->state == TCPIP_TCP_SOCK_STATE_CLOSED)
            return ENOCONNECT;
    }
    
    /* return success!*/
    return EOK;
}

/* establish the connection to the remote party */
err_t TCPIPTcpSock_Connect(tcpip_tcp_sock_t *sock, tcpip_ip_addr_t ip,
    tcpip_tcp_port_t port, dtime_t timeout)
{
    /* timestamp */
    time_t ts = time(0);

    /* connect called on socket that is already connected to the destination
     * ip/port */
    if (sock->state == TCPIP_TCP_SOCK_STATE_CONNECT &&
        sock->rem_port == port &&
        TCPIPIpAddr_AddressMatch(ip, sock->addr))
        return EOK;

    /* invalid socket provided */
    if (!(sock->state == TCPIP_TCP_SOCK_STATE_CLOSED))
        return EARGVAL;
    /* port number must be non zero */
    if (!port)
        return EARGVAL;

    /* reset control flags */
    sock->tx_flags = sock->rx_flags = 0;
    /* store connection credentials */
    sock->rem_port = port, sock->addr = ip;
    /* generate pseudo random port number */
    sock->loc_port = (loc_port_num = (loc_port_num < 10000 ?
        10000 : loc_port_num + 1));
    /* initiate sequence numbers */
    sock->tx_seq_start = sock->tx_seq_init = time(0);

    /* move the socket to connect state to cause SYN frame to be sent */
    sock->state = TCPIP_TCP_SOCK_STATE_CONNECT;
    /* close both sides of the link */
    sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSED;
    sock->rem_link = TCPIP_TCP_LINK_STATE_CLOSED;
    /* clear the queues */
    Queue_Drop(sock->rxq, Queue_GetUsed(sock->rxq));
    Queue_Drop(sock->txq, Queue_GetUsed(sock->txq));

    /* wait for someone to establish connection */
    while (sock->state != TCPIP_TCP_SOCK_STATE_ESTABLISHED) {
        /* timeout has occured? */
        if (timeout && dtime(time(0), ts) >= timeout)
            sock->state = TCPIP_TCP_SOCK_STATE_CLOSED;
        /* connection not established ;-( */
        if (sock->state == TCPIP_TCP_SOCK_STATE_CLOSED)
            return ENOCONNECT;
        /* still waiting */
        Yield();
    }

    /* we are connected */
    return EOK;
}

/* receive data from socket */
err_t TCPIPTcpSock_Recv(tcpip_tcp_sock_t *sock, void *ptr, size_t size, 
    dtime_t timeout)
{
    /* current timestamp, number of bytes read from the rx buffer */
    time_t ts = time(0); size_t b_read;

    /* poll as long as there is no data stored in the rx buffer */
    while (!(b_read = Queue_Get(sock->rxq, ptr, size))) {
        /* timeout support */
        if (timeout && dtime(time(0), ts) > timeout)
            return ETIMEOUT;
        /* disconnect support */
        if (sock->state != TCPIP_TCP_SOCK_STATE_ESTABLISHED)
            return ENOCONNECT;
        /* wait for data to come */
        Yield();
    }

    /* report the number of bytes read */
    return b_read;
}

/* send to remote party */
err_t TCPIPTcpSock_Send(tcpip_tcp_sock_t *sock, const void *ptr, size_t size, 
    dtime_t timeout)
{
    /* current timestamp */
    time_t ts = time(0);
    /* number of bytes stored in single transaction */
    size_t b_stored, b_written = 0;

    /* data is pushed in chunks if it's larger than the tx buffer */
    do {
        /* timeout support */
        if (timeout && dtime(time(0), ts) > timeout)
            return ETIMEOUT;
        /* disconnect support */
        if (sock->state != TCPIP_TCP_SOCK_STATE_ESTABLISHED)
            return ENOCONNECT;
        /* write next chunk of data into buffer */
        b_stored = Queue_Put(sock->txq, 
            (const uint8_t *)ptr + b_written, size - b_written);
        /* not all data was sent? */
        if ((b_written += b_stored) < size)
            Yield();
    /* still some data left to be sent? */
    } while (b_written != size);

    /* return the total number of bytes sent */
    return b_written;
}

/* close the socket */
err_t TCPIPTcpSock_Close(tcpip_tcp_sock_t *sock, dtime_t timeout)
{
    /* current timestamp */
    time_t ts = time(0);

    /* socket in weird state */
    if ((sock->state == TCPIP_TCP_SOCK_STATE_FREE) ||
        (sock->state == TCPIP_TCP_SOCK_STATE_LISTEN))
        return EARGVAL;

    /* flush the data that remains in output buffers if in open state */
    while (sock->state == TCPIP_TCP_SOCK_STATE_ESTABLISHED &&
           Queue_GetUsed(sock->txq) != 0) {
        /* timeout support */
        if (timeout && dtime(time(0), ts) > timeout) {
            sock->state = TCPIP_TCP_SOCK_STATE_CLOSED; return ETIMEOUT;
        }
        /* still waiting */
        Yield();
    }

    /* close our site of the connection */
    if (sock->loc_link == TCPIP_TCP_LINK_STATE_OPEN) {
        sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSING;
        sock->state = TCPIP_TCP_SOCK_STATE_CLOSING;
        sock->syn_fin_ts = time(0);
    }
    /* wait for the closure */
    while (sock->state != TCPIP_TCP_SOCK_STATE_CLOSED) {
        /* timeout support */
        if (timeout && dtime(time(0), ts) > timeout) {
            sock->state = TCPIP_TCP_SOCK_STATE_CLOSED; return ETIMEOUT;
        }
        /* still waiting */
        Yield();
    }

    /* return success */
    return EOK;
}