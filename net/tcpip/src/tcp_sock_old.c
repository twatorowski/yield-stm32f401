/**
 * @file tcp_sock.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-05
 * 
 * @brief  TCP/IP Stack: TCP Sockets
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
    /* processing status */
    err_t ec = EFATAL;
    /* packet header */
    tcpip_tcp_frame_t *tcp = frame->tcp;

    /* socket is closed or free */
    if (sock->state == TCPIP_TCP_SOCK_STATE_CLOSED ||
        sock->state == TCPIP_TCP_SOCK_STATE_FREE)
        goto end;

    /* extract port numbers */
    tcpip_tcp_port_t src_port = TCPIPTcpFrame_GetSrcPort(tcp);
    tcpip_tcp_port_t dst_port = TCPIPTcpFrame_GetDstPort(tcp);
    /* destination port mismatch */
    if (sock->loc_port != dst_port)
        goto end;
    
    /* get the source ip address */
    tcpip_ip_addr_t ip = TCPIPIpFrame_GetSrcAddr(frame->ip);
    /* at states above 'listen' the ip and  source port must match */
    if ((sock->state != TCPIP_TCP_SOCK_STATE_LISTEN) && 
         (sock->rem_port != src_port || 
         !TCPIPIpAddr_AddressMatch(ip, sock->addr)))
        goto end;

    /* get the sequence numbers as well */
    uint32_t seq = TCPIPTcpFrame_GetSeq(tcp);
    uint32_t ack = TCPIPTcpFrame_GetAck(tcp);
    /* and the window size */
    uint32_t win = TCPIPTcpFrame_GetWindow(tcp);
    /* extract flags */
    tcpip_tcp_flags_t rx_flags = TCPIPTcpFrame_GetFlags(tcp);

    /* frame shall be accepted */
    ec = EOK;

    /* syn flag support */
    if (rx_flags & TCPIP_TCP_FLAGS_SYN) {
        /* syn flag received in listen state indicates that someone is trying
         * to connect */
        if (sock->state == TCPIP_TCP_SOCK_STATE_LISTEN) {
            /* store the credentials */
            sock->addr = ip, sock->rem_port = src_port;
            /* along with the syn segment connection initiator sends it's
             * sequencing numbers */
            sock->rx_seq_recvd = seq + 1;
            sock->rx_seq_acked = seq;
            /* this is a perfect spot to initiate our sequence numbers */
            sock->tx_seq_start = time(0);
            sock->tx_win = win;
        }
        /* if we are the initiating side of the connection then we expect
         * syn+ack frame to be recevied */
        if (sock->state == TCPIP_TCP_SOCK_STATE_CONNECT &&
            rx_flags & TCPIP_TCP_FLAGS_ACK) {
            /* store the remote sequencing */
            sock->rx_seq_recvd = seq + 1;
            sock->rx_seq_acked = seq;
            /* and remote window size */
            sock->tx_win = win;
        }
    }

    /* frame carries valid value in the ack field? */
    if (rx_flags & TCPIP_TCP_FLAGS_ACK) {
        /* special flags move the sequencing numbers by one w.r.t data */
        tcpip_frame_flags_t sf = TCPIP_TCP_FLAGS_FIN | TCPIP_TCP_FLAGS_SYN;

        /* incoming data in sequence? */
        if (seq == sock->rx_seq_recvd) {
            /* special flags increment the sequence received */
            if (rx_flags & sf)
                sock->rx_seq_recvd += 1;

            /* store the incoming data */
            size_t b_stored = Queue_Put(sock->rxq, frame->ptr, frame->size);
            /* update the sequence number according to accepted data size */
            sock->rx_seq_recvd = sock->rx_seq_recvd + b_stored;
        }

        /* check if remote site received our data */
        if (ack == sock->tx_seq_end) {
            /* let's see how many bytes are acknowledged */
            size_t b_acked = ack - sock->tx_seq_start;
            /* handle special flags */
            if (sock->tx_flags & sf)
                b_acked -= 1;

            /* drop the data from the queue */
            Queue_Drop(sock->txq, b_acked);
            /* move the sequence numbers */
            sock->tx_seq_start = sock->tx_seq_end;
            /* valid ack received, we can reset the retransmission counter */
            sock->tx_retr = 0;
        /* frame not in sequence, remove the ack flag */
        } else {
            rx_flags &= ~TCPIP_TCP_FLAGS_ACK;
        }
    }

    /* store the reception flags */
    sock->rx_flags = rx_flags;
    /* frame not accepted by this socket */
    end: return ec;
}

/* process outgoing traffic */
static void TCPIPTcpSock_ProcessOutgoing(tcpip_tcp_sock_t *sock)
{
    /* frame buffer */
    tcpip_frame_t frame;
    /* this variable will denote state changes */
    enum tcpip_tcp_sock_state next_state;
    /* frame flags */
    tcpip_tcp_flags_t tx_flags = 0;


    /* initial assumption that the state will not change */
    next_state = sock->state;
    /* state transisions and flag fields determination */
    fsm_again: switch (sock->state) {
    /* waiting for the remote party to connect? */
    case TCPIP_TCP_SOCK_STATE_LISTEN : {
        /* segment with syn flag was received? */
        if (sock->rx_flags == TCPIP_TCP_FLAGS_SYN) {
            next_state = TCPIP_TCP_SOCK_STATE_ESTABLISHING;
        /* no syn, no point in processing */
        } else {
            goto end;
        }
    } break;
    /* user wants to connect to remote party? */
    case TCPIP_TCP_SOCK_STATE_CONNECT : {
        /* start emitting syn segments */
        tx_flags = TCPIP_TCP_FLAGS_SYN;
        /* connection reset */
        if (sock->rx_flags & TCPIP_TCP_FLAGS_RST)
            next_state = TCPIP_TCP_SOCK_STATE_RESET;
        /* syn request sent, ack received? */
        if (sock->tx_flags == tx_flags &&
            sock->rx_flags == (TCPIP_TCP_FLAGS_ACK | TCPIP_TCP_FLAGS_SYN)) {
            /* mark links as open */
            sock->loc_link = TCPIP_TCP_LINK_STATE_OPEN;
            sock->rem_link = TCPIP_TCP_LINK_STATE_OPEN;
            /* advance the state machine */
            next_state = TCPIP_TCP_SOCK_STATE_ESTABLISHED;
        }
    } break;
    /* during connection establishment? */
    case TCPIP_TCP_SOCK_STATE_ESTABLISHING : {
        /* normal flags for connection establishment are syn+ack */
        tx_flags = TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_ACK;
        /* got the response in form of ack? */
        if (sock->tx_flags == tx_flags &&
            sock->rx_flags == TCPIP_TCP_FLAGS_ACK) {
            /* mark links as open */
            sock->loc_link = TCPIP_TCP_LINK_STATE_OPEN;
            sock->rem_link = TCPIP_TCP_LINK_STATE_OPEN;
            /* go to established state */
            next_state = TCPIP_TCP_SOCK_STATE_ESTABLISHED;
        }
    } break;
    /* connection is now established */
    case TCPIP_TCP_SOCK_STATE_ESTABLISHED :{
        /* during normal activity all segments shall have the ack flag set */
        tx_flags = TCPIP_TCP_FLAGS_ACK;
        /* got a fin segment? */
        if (sock->rx_flags & TCPIP_TCP_FLAGS_FIN)
            next_state = TCPIP_TCP_SOCK_STATE_CLOSING;
        /* connection reset by peer */
        if (sock->rx_flags & TCPIP_TCP_FLAGS_RST)
            next_state = TCPIP_TCP_SOCK_STATE_RESET;
    } break;
    /* connection is closing */
    case TCPIP_TCP_SOCK_STATE_CLOSING: {
        /* send segments with fin flag set */
        tx_flags = TCPIP_TCP_FLAGS_ACK | TCPIP_TCP_FLAGS_FIN;
        /* link brutally terminated? */
        if (sock->rx_flags & TCPIP_TCP_FLAGS_RST)
            next_state = TCPIP_TCP_SOCK_STATE_RESET;
        /* remote site terminated the link? */
        if (sock->rx_flags & TCPIP_TCP_FLAGS_FIN)
            sock->rem_link = TCPIP_TCP_LINK_STATE_CLOSED;
        /* ve have received the ack to our fin? */
        if (sock->tx_flags & TCPIP_TCP_FLAGS_FIN &&
            sock->rx_flags & TCPIP_TCP_FLAGS_ACK)
            sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSED;
        /* both sides of the link are now closed? */
        if (sock->rem_link == TCPIP_TCP_LINK_STATE_CLOSED &&
            sock->loc_link == TCPIP_TCP_LINK_STATE_CLOSED)
            next_state = TCPIP_TCP_SOCK_STATE_CLOSED;
    } break;
    /* connection reset */
    case TCPIP_TCP_SOCK_STATE_RESET : {
        /* both link sites are now closed */
        sock->rem_link = TCPIP_TCP_LINK_STATE_CLOSED;
        sock->loc_link = TCPIP_TCP_LINK_STATE_CLOSED;
        /* close the socket */
        next_state = TCPIP_TCP_SOCK_STATE_CLOSED;
    } break;
    /* connection is now free */
    case TCPIP_TCP_SOCK_STATE_CLOSED:
    case TCPIP_TCP_SOCK_STATE_FREE: {
        goto end;
    } break;
    }

    /* state has changed- redo the state machine */
    if (next_state != sock->state) {
        sock->state = next_state; goto fsm_again;
    /* fsm processing is complete, we can consume the rx flags and prepare the
     * tx flags */
    } else {
        sock->tx_flags = tx_flags, sock->rx_flags = 0;
    }

    /* socket is not used */
    if (sock->state == TCPIP_TCP_SOCK_STATE_FREE ||
        sock->state == TCPIP_TCP_SOCK_STATE_CLOSED ||
        sock->state == TCPIP_TCP_SOCK_STATE_LISTEN)
        goto end;

    /* these are the conditions that start the tx procedure:
     * special flags OR data to be set OR need to ack the data received */
    if (((sock->tx_flags & (TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_FIN)) == 0) &&
        (Queue_GetUsed(sock->txq) == 0) && 
        (sock->rx_seq_recvd == sock->rx_seq_acked))
        goto end;

    /* check if enough time has passed since last retransmission */
    if ((dtime(time(0), sock->tx_ts) < 300 * sock->tx_retr))
        goto end;

    /* allocate space for frame to be sent */
    if (TCPIPTcp_Alloc(&frame) != EOK)
        goto end;

    /* copy data to the frame payload section */ 
    frame.size = Queue_Peek(sock->txq, frame.ptr, frame.size);
    /* ensure that the data is pushed to the application on the remote site */
    if (frame.size > 0)
        sock->tx_flags |= TCPIP_TCP_FLAGS_PSH;
    /* prepare the sequencing number denoting the end of the message */
    sock->tx_seq_end = sock->tx_seq_start + frame.size;
    /* are we sending any of the special flags? */
    if (sock->tx_flags & (TCPIP_TCP_FLAGS_SYN | TCPIP_TCP_FLAGS_FIN))
        sock->tx_seq_end += 1; 
    
    /* build up the frame fields */
    uint32_t seq = sock->tx_seq_start;
    uint32_t ack = sock->rx_seq_recvd;
    uint16_t win = Queue_GetFree(sock->rxq);

    /* try to send the frame */
    if (TCPIPTcp_Send(&frame, sock->addr, sock->loc_port,
        sock->rem_port, seq, ack, win, sock->tx_flags) < EOK)
        goto end;

    /* if the send was successful then we can move the rx ack numbers */
    sock->rx_seq_acked = sock->rx_seq_recvd;
    /* frame that was sent had payload besides the ack */
    if (sock->tx_seq_end != sock->tx_seq_start)
        sock->tx_retr += 1, sock->tx_ts = time(0);

    /* end of processing */
    end: return;
}

/* socket housekeeping task */
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
    Yield_CreateTask(TCPIPTcpSock_Output, 0, 1024);
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

    /* wasn't able to process the frame, reject it */
    return ec == EOK ? EOK : TCPIPTcpSock_Reject(frame);
}
//TODO: 192.168.100.110:1234
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
    assert(sock->rxq && sock->txq, "unable to allocate socket memory", 0);
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
    while (sock->state != TCPIP_TCP_SOCK_STATE_ESTABLISHED)
        Yield();
    
    /* return success!*/
    return EOK;
}

/* establish the connection to the remote party */
err_t TCPIPTcpSock_Connect(tcpip_tcp_sock_t *sock, tcpip_ip_addr_t ip,
    tcpip_tcp_port_t port, dtime_t timeout)
{
    /* timestamp */
    time_t ts = time(0);

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