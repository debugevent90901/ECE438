#ifndef UTILITY_H
#define UTILITY_H

// #define DEBUG

#define MSS         1024
#define RTT         20
#define RTO         (2*RTT)

enum pkt_type {ACK, DATA, SYN, SYNACK, FIN, FINACK, HOLDER};

typedef struct packet {
    pkt_type    type;
    int         seq_num;
    int         nbyte;
    char        data[MSS];
} packet_t;

typedef struct ack_type {
    pkt_type    type;
    int         seq_num;
} ack_t;

#endif