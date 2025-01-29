#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "packet.h"

#define NET_PROTO_HANDSHAKE_C 0
#define NET_PROTO_HANDSHAKE_S 1
#define NET_PROTO_PERSON 2
#define NET_PROTO_MESSAGE 3
#define NET_PROTO_INFO_C 4

#define NET_PINFO_AUDIENCE 1
#define NET_PINFO_HISTORY 2

/*
Below are all packets that can be sent over the network. It is assumed
that all packet structures own the data that they may point to.
*/

/*client only packets*/
struct protocol_packet_handshake_c {
    unsigned long proto_ver;
    unsigned long proto_flags;
};

struct protocol_packet_info_c {
    unsigned long info_type;
};

/*server only packets*/
struct protocol_packet_handshake_s {
    unsigned long proto_ver;
    unsigned long proto_flags;
    unsigned long self_id;
};

/*any side packets*/
struct protocol_packet_person {
    unsigned long person_id;
    char *name;
};

struct protocol_packet_message {
    unsigned long person_id;
    unsigned long encryption;
    long int index;
    char *message;
};
/*end packets*/

struct protocol_packet {
    long int type;
    union protocol_packet_union {
        struct protocol_packet_handshake_c handshake_c;
        struct protocol_packet_info_c info_c;
        struct protocol_packet_handshake_s handshake_s;
        struct protocol_packet_person person;
        struct protocol_packet_message message;
    } as;
};

parseResult packet_serialize(net_buffer_t *protocol_packet,
                             const struct protocol_packet *data);
parseResult packet_deserialize(net_buffer_t *protocol_packet,
                               struct protocol_packet *res);

#endif /* PROTOCOL_H_ */
