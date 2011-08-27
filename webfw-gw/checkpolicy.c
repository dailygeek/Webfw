#include <stdlib.h>
#include <sqlite.h>
#include <glib.h>

#include "header.h"

typedef struct sqlite_vm sqlite_vm;

int sqlite_compile(
  sqlite *db,              /* The open database */
  const char *zSql,        /* SQL statement to be compiled */
  const char **pzTail,     /* OUT: uncompiled tail of zSql */
  sqlite_vm **ppVm,        /* OUT: the virtual machine to execute zSql */
  char **pzErrmsg          /* OUT: Error message. */
);

int sqlite_step(
  sqlite_vm *pVm,          /* The virtual machine to execute */
  int *pN,                 /* OUT: Number of columns in result */
  const char ***pazValue,  /* OUT: Column data */
  const char ***pazColName /* OUT: Column names and datatypes */
);

int sqlite_finalize(
  sqlite_vm *pVm,          /* The virtual machine to be finalized */
  char **pzErrMsg          /* OUT: Error message */
);

gboolean checkpolicy(struct value_struct *connection, gchar* username)
{
	sqlite *policydb;
	sqlite_vm *dbcursor;
    char *dberr = 0;
    const char *sqltail;
	gboolean retval = FALSE;
	gchar *querry;
	gchar *s_protocol;
	
	g_debug("Checking Policy entries for user: %s",connection->username);
	
	if( connection->protocol == 6)
		s_protocol = "TCP";
	else if ( connection->protocol == 17 )
		s_protocol = "UDP";
	else
		s_protocol = "ICMP";
	
	uerry = g_strdup_printf("select * from user_policy where username=\"%s\" and dst_ip=\"%s\" and dst_port = \"%i\" and prot = \"%s\";",connection->username,
    													connection->s_dst_ip,
    													connection->dst_port,
														s_protocol);
	
  	policydb = sqlite_open( "/root/htfirewall.sdb" , 0 , NULL);
  	
  	if(SQLITE_OK != sqlite_compile(policydb, querry, &sqltail, &dbcursor, &dberr)) {
    	g_debug("errorf: %s", dberr);
		free(dberr);
	}else{
		g_debug("querry OK");
		int ncols;
		int rows = 0;
		const char **colnames;
		const char **values;
		
		while( SQLITE_ROW == sqlite_step(dbcursor, &ncols, &values, &colnames)) {
			rows++;
		}
		if ( rows==1)
			retval = TRUE;
	}
  	sqlite_close(policydb);
 	g_free(querry);
 	g_free(s_protocol);
 	return retval;
}