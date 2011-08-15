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
#include <glib.h>
#include <curl/curl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#define DAEMON_NAME "webfw"
#define PID_FILE "/var/run/webfw.pid"

struct value_struct{
	char s_src_ip[INET_ADDRSTRLEN]; // source address as string
        char s_dst_ip[INET_ADDRSTRLEN]; // destination address as string
	guint32 src_ip; //src address in network byte order
  	guint32 dst_ip; // destination address in network byte order
    	guint16 dst_port; //destination port to identify services 
  	guint16 protocol; //connection protocol
}; // used for whitelist and blacklist values

struct handle_struct {
  GString *url;                 // where to get list from web
  //GString *current_csv;   // copy of the last list download
  GHashTable *blacklist;
  guint64 bl_max_timediff;
  GHashTable *whitelist;
  guint64 wl_max_timediff;
  guint64 last_sec;
  guint64 timeout;
};

#define MINIMUM_TIMEOUT 9

static struct nfq_handle *h = NULL;
static struct nfq_q_handle *qh = NULL;

static unsigned char bfr[32768];

int cb (struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data);

void listman_add2list(GHashTable *list,struct value_struct *connection, guint64 *val);
gint listman_clean_list(GHashTable *list,guint64 now, guint64 max_timediff);
void listman_bl2wl(struct handle_struct *handle,guint key);
gint listman_init(struct handle_struct *handle);
gboolean listman_check_key(GHashTable *list,struct value_struct *connection,guint64 now, guint64 timeout);

gboolean fetch_entry(const gchar *url, struct value_struct *connection);
