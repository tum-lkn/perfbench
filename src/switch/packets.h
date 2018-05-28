/* Copyright (c) 2008 The Board of Trustees of The Leland Stanford
 * Junior University
 *
 * We are making the OpenFlow specification and associated documentation
 * (Software) available for public use and benefit with the expectation
 * that others will use, modify and enhance the Software and contribute
 * those enhancements back to the community. However, since we would
 * like to make the Software available for broadest use, with as few
 * restrictions as possible permission is hereby granted, free of
 * charge, to any person obtaining a copy of this Software to deal in
 * the Software under the copyrights without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The name and trademarks of copyright holder(s) may NOT be used in
 * advertising or publicity pertaining to the Software or any
 * derivatives without specific, written prior permission.
 */
#ifndef SW_PACKETS_H
#define SW_PACKETS_H 1

#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>

#define ETH_ADDR_LEN           6


#define ETH_TYPE_II_START      0x0600
#define ETH_TYPE_IP            0x0800
#define ETH_TYPE_IPV6          0x86dd
#define ETH_TYPE_ARP           0x0806
#define ETH_TYPE_VLAN          0x8100
#define ETH_TYPE_SVLAN         0x9200
#define ETH_TYPE_VLAN_QinQ     0x9100
#define ETH_TYPE_VLAN_PBB_B    0x88a8
#define ETH_TYPE_VLAN_PBB_S    0x88e7
#define ETH_TYPE_MPLS          0x8847
#define ETH_TYPE_MPLS_MCAST    0x8848

#define ETH_HEADER_LEN 14
#define ETH_PAYLOAD_MIN 46
#define ETH_PAYLOAD_MAX 1500
#define ETH_TOTAL_MIN (ETH_HEADER_LEN + ETH_PAYLOAD_MIN)
#define ETH_TOTAL_MAX (ETH_HEADER_LEN + ETH_PAYLOAD_MAX)
#define ETH_VLAN_TOTAL_MAX (ETH_HEADER_LEN + VLAN_HEADER_LEN + ETH_PAYLOAD_MAX)
struct eth_header {
    uint8_t eth_dst[ETH_ADDR_LEN];
    uint8_t eth_src[ETH_ADDR_LEN];
    uint16_t eth_type;
} __attribute__((packed));

#define LLC_DSAP_SNAP 0xaa
#define LLC_SSAP_SNAP 0xaa
#define LLC_CNTL_SNAP 3

#define LLC_HEADER_LEN 3
struct llc_header {
    uint8_t llc_dsap;
    uint8_t llc_ssap;
    uint8_t llc_cntl;
} __attribute__((packed));

#define SNAP_ORG_ETHERNET "\0\0" /* The compiler adds a null byte, so
                                    sizeof(SNAP_ORG_ETHERNET) == 3. */
#define SNAP_HEADER_LEN 5
struct snap_header {
    uint8_t snap_org[3];
    uint16_t snap_type;
} __attribute__((packed));

#define LLC_SNAP_HEADER_LEN (LLC_HEADER_LEN + SNAP_HEADER_LEN)
struct llc_snap_header {
    struct llc_header llc;
    struct snap_header snap;
} __attribute__((packed));

#define VLAN_VID_MASK 0x0fff
#define VLAN_VID_SHIFT 0
#define VLAN_PCP_MASK 0xe000
#define VLAN_PCP_SHIFT 13
#define VLAN_PCP_BITMASK 0x0007 /* the least 3-bit is valid */

#define VLAN_VID_MAX 4095
#define VLAN_PCP_MAX 7

#define VLAN_HEADER_LEN 4
struct vlan_header {
    uint16_t vlan_tci;          /* Lowest 12 bits are VLAN ID. */
    uint16_t vlan_next_type;
};

/*This structure is used to reconstruct the vlan tag on netdev*/
struct vlan_tag {
    uint16_t vlan_tp_id;          /* Lowest 12 bits are VLAN ID. */
    uint16_t vlan_tci;
};

#define VLAN_ETH_HEADER_LEN (ETH_HEADER_LEN + VLAN_HEADER_LEN)
struct vlan_eth_header {
    uint8_t veth_dst[ETH_ADDR_LEN];
    uint8_t veth_src[ETH_ADDR_LEN];
    uint16_t veth_type;         /* Always hton16(ETH_TYPE_VLAN). */
    uint16_t veth_tci;          /* Lowest 12 bits are VLAN ID. */
    uint16_t veth_next_type;
} __attribute__((packed));

/* The "(void) (ip)[0]" below has no effect on the value, since it's the first
 * argument of a comma expression, but it makes sure that 'ip' is a pointer.
 * This is useful since a common mistake is to pass an integer instead of a
 * pointer to IP_ARGS. */
#define IP_ARGS(ip)                             \
        ((void) (ip)[0], ((uint8_t *) ip)[0]),  \
        ((uint8_t *) ip)[1],                    \
        ((uint8_t *) ip)[2],                    \
        ((uint8_t *) ip)[3]

#define IP_VER(ip_ihl_ver) ((ip_ihl_ver) >> 4)
#define IP_IHL(ip_ihl_ver) ((ip_ihl_ver) & 15)
#define IP_IHL_VER(ihl, ver) (((ver) << 4) | (ihl))

/* TOS fields. */
#define IP_ECN_MASK 0x03
#define IP_DSCP_MASK 0xfc


#define IP_TYPE_ICMP   1
#define IP_TYPE_TCP    6
#define IP_TYPE_UDP   17
#define IP_TYPE_SCTP 132

#define IP_VERSION 4

#define IP_DONT_FRAGMENT  0x4000 /* Don't fragment. */
#define IP_MORE_FRAGMENTS 0x2000 /* More fragments. */
#define IP_FRAG_OFF_MASK  0x1fff /* Fragment offset. */
#define IP_IS_FRAGMENT(ip_frag_off) \
        ((ip_frag_off) & hton16(IP_MORE_FRAGMENTS | IP_FRAG_OFF_MASK))

#define IP_ADDR_LEN 4

#define IP_HEADER_LEN 20
struct ip_header {
    uint8_t ip_ihl_ver;
    uint8_t ip_tos;
    uint16_t ip_tot_len;
    uint16_t ip_id;
    uint16_t ip_frag_off;
    uint8_t ip_ttl;
    uint8_t ip_proto;
    uint16_t ip_csum;
    uint32_t ip_src;
    uint32_t ip_dst;
};

#define UDP_HEADER_LEN 8
struct udp_header {
    uint16_t udp_src;
    uint16_t udp_dst;
    uint16_t udp_len;
    uint16_t udp_csum;
};

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

#define TCP_FLAGS(tcp_ctl) (hton16(tcp_ctl) & 0x003f)
#define TCP_OFFSET(tcp_ctl) (hton16(tcp_ctl) >> 12)

#define TCP_HEADER_LEN 20
struct tcp_header {
    uint16_t tcp_src;
    uint16_t tcp_dst;
    uint32_t tcp_seq;
    uint32_t tcp_ack;
    uint16_t tcp_ctl;
    uint16_t tcp_winsz;
    uint16_t tcp_csum;
    uint16_t tcp_urg;
};

#define SCTP_HEADER_LEN 12
struct sctp_header {
    uint16_t sctp_src;
    uint16_t sctp_dst;
    uint32_t sctp_verif;
    uint32_t sctp_csum;
};

#define ICMP_HEADER_LEN 4
struct icmp_header {
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t icmp_csum;
};

#define ARP_HRD_ETHERNET 1
#define ARP_PRO_IP 0x0800
#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

#define ARP_ETH_HEADER_LEN 28
struct arp_eth_header {
    /* Generic members. */
    uint16_t ar_hrd;           /* Hardware type. */
    uint16_t ar_pro;           /* Protocol type. */
    uint8_t ar_hln;            /* Hardware address length. */
    uint8_t ar_pln;            /* Protocol address length. */
    uint16_t ar_op;            /* Opcode. */

    /* Ethernet+IPv4 specific members. */
    uint8_t ar_sha[ETH_ADDR_LEN]; /* Sender hardware address. */
    uint32_t ar_spa;           /* Sender protocol address. */
    uint8_t ar_tha[ETH_ADDR_LEN]; /* Target hardware address. */
    uint32_t ar_tpa;           /* Target protocol address. */
} __attribute__((packed));

struct qtag_prefix {
    uint16_t eth_type;      /* ETH_TYPE_VLAN */
    uint16_t tci;
};

struct protocols_std {
   struct eth_header      * eth;
   struct snap_header     * eth_snap; /* points to SNAP header if eth is 802.3 */
   struct vlan_header     * vlan;
   struct vlan_header     * vlan_last; /* points to the last VLAN header */
   struct mpls_header     * mpls;
   struct pbb_header      * pbb;
   struct ip_header       * ipv4;
   struct ipv6_header     * ipv6;
   struct arp_eth_header  * arp;
   struct tcp_header      * tcp;
   struct udp_header      * udp;
   struct sctp_header     * sctp;
   struct icmp_header     * icmp;
};

static inline void
protocol_reset(struct protocols_std *proto) {
    proto->eth       = NULL;
    proto->eth_snap  = NULL;
    proto->vlan      = NULL;
    proto->vlan_last = NULL;
    proto->mpls      = NULL;
    proto->ipv4      = NULL;
    proto->ipv6      = NULL;
    proto->arp       = NULL;
    proto->tcp       = NULL;
    proto->udp       = NULL;
    proto->sctp      = NULL;
    proto->icmp      = NULL;
    proto->pbb       = NULL;
}

struct packet {
    uint16_t in_port;
    uint8_t* data;
    size_t len;
};


#endif /* packets.h */
