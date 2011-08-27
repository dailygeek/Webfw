#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "header.h"

static int
myworker (struct handle_struct *handle, void *packet, int len, struct timeval *tv){
  struct iphdr *i;
  struct in_addr x;
  struct in_addr y;
  struct tcphdr *tcph;
  struct udphdr *udph;
  struct value_struct connection;

  //guint64 val=(guint64)tv->tv_sec; Have not been able to gain a timestamp from any paket so far!
  time_t now = time (NULL);
  guint64 val=(guint64)now;  // using system time when paket arrives instead

  i = (struct iphdr *) (packet);
  connection.protocol = i->protocol;

  if( connection.protocol == 6 ){
        tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
        connection.dst_port = ntohs(tcph->dest);
  }
  if( connection.protocol == 17 ){
        udph = (struct udphdr *)(packet + sizeof(struct iphdr));
        connection.dst_port = ntohs(udph->dest);
  }
  if (len < sizeof (struct iphdr))
    return -1;
  if (len != ntohs(i->tot_len))
    return -1;

  x.s_addr = i->saddr;
  y.s_addr = i->daddr;
  connection.src_ip = i->saddr;
  connection.dst_ip = i->daddr;

  inet_ntop(AF_INET, &x, connection.s_src_ip, sizeof(connection.s_src_ip));
  inet_ntop(AF_INET, &y, connection.s_dst_ip, sizeof(connection.s_dst_ip));

  g_debug("Source: %s \n",connection.s_src_ip);
  g_debug("Destination: %s \n",connection.s_dst_ip);
  g_debug("Source: %i \n",connection.src_ip);
  g_debug("Destination: %i \n",connection.dst_ip);
  g_debug("Protocol: %i \n",connection.protocol);  /* 1= ICMP, 6= TCP, 17=UDP */
  g_debug("Destination Port: %i\n",connection.dst_port);

  /* in whitelist? */
  if (TRUE == listman_check_key(handle->whitelist,&connection,val,handle->timeout)){
          g_debug("in whitelist");
          return 0;
  }
  /* already in blacklist? */
  if (FALSE == listman_check_key(handle->blacklist,&connection,val,handle->timeout)){
          if ( TRUE == fetch_entry(handle->url->str,&connection)){
                g_debug("recieved user: %s",connection.username);
                g_debug("Calling Check Policy Function");
                if (TRUE == checkpolicy(&connection)){
                        listman_add2list(handle->whitelist,&connection,&val);
                        return 0;
                }
         } else {
                listman_add2list(handle->blacklist,&connection,&val);
          }
  }
  g_debug("in blacklist");
  return -1;
}

int cb (struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
    struct nfq_data *nfa, void *data)
{
  int id;
  int len;
  int hlen;
  char *pkt;
  struct nfqnl_msg_packet_hdr *ph;
  struct nfqnl_msg_packet_hw *hwph;
  struct timeval tv;

  tv.tv_sec = 0; // init

  if ((ph = nfq_get_msg_packet_hdr(nfa)))
    id = ntohl(ph->packet_id);
  else
    id = 0;

  if ((hwph = nfq_get_packet_hw(nfa)))
    hlen = ntohs(hwph->hw_addrlen);
  else
    hlen = 0;

  /* hole payload ... falls es nicht klappt ... Paket durchreichen */
  if ((len = nfq_get_payload (nfa, (char **) &pkt)) < 0)
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);

  nfq_get_timestamp(nfa,&tv);
  return nfq_set_verdict(qh, id, myworker (data, pkt, len,&tv) ? NF_DROP :
                          NF_ACCEPT, 0, NULL);
}

