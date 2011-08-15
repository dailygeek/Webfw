#include "header.h"

void listman_add2list(GHashTable *list,struct value_struct *connection, guint64 *val){
	int hash; // store calculated hash for connection
	hash  = ((size_t)(connection->src_ip) * 59) ^ ((size_t)(connection->dst_ip)) ^ ((size_t)(connection->dst_port << 16 )) ^ ((size_t)(connection->protocol));
	int  *keycopy;
	guint64 *valcopy;
	keycopy = g_memdup(&hash,sizeof(*keycopy));
	valcopy = g_memdup(val,sizeof(*valcopy));
	g_hash_table_insert(list,keycopy,valcopy);
	g_debug("calculated hashkey: %i",hash);
}

gint listman_init(struct handle_struct *handle){
	 //handle->blacklist = g_hash_table_new(g_int_hash,g_int_equal);
	 handle->blacklist = g_hash_table_new_full(g_int_hash,g_int_equal,NULL, g_free);
	 g_assert(handle->blacklist);
	 handle->bl_max_timediff = 60*10; // 10 Minuten
	 	 
	 //handle->whitelist = g_hash_table_new(g_int_hash,g_int_equal);
	 handle->whitelist = g_hash_table_new_full(g_int_hash,g_int_equal,NULL, g_free);
	 g_assert(handle->whitelist);
	 handle->wl_max_timediff = 100; // 100 Seconds
	 
	 return 0;
}

/**
 * return TRUE if in and does update of timestamp aka value
 */
gboolean listman_check_key(GHashTable *list,struct value_struct *connection,guint64 now,guint64 timeout){
   gpointer k;
   guint64 *v;
   guint64 timediff;
   int hash;
   hash  = ((size_t)(connection->src_ip) * 59) ^ ((size_t)(connection->dst_ip)) ^ ((size_t)(connection->dst_port << 16 )) ^ ((size_t)(connection->protocol));
    
    if (TRUE == g_hash_table_lookup_extended(list,&hash,&k,(gpointer *)&v)){
		timediff = now - *v;
		if ( timediff <= timeout ){
			g_debug("timeout not reached yet %i",timediff);
			return TRUE;
		}
		else {
			g_debug("timediff bigger than timeout! %i",timediff);
			g_debug("deleting entry from list");
			g_hash_table_remove(list,&hash);
		}	
	}
	return FALSE;
}
