/* -*- Mode:C; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "header.h"

void deinit_nfq(){
  nfq_destroy_queue(qh);
  nfq_close(h);
}


void die (void)
{
  if (h)
    //nfq_close (h);
    deinit_nfq();
    remove(PID_FILE);
  exit (EXIT_FAILURE);
}

gint init_nfq(struct handle_struct *handle){
  if ((h = nfq_open()) == NULL)
    {
      fprintf(stderr, "No nfqueue handle, aborting.n");
      die();
    }

  if (nfq_unbind_pf(h, AF_INET) < 0)
    {
      fprintf(stderr, "No nfqueue unbind, aborting.n");
      die();
    }

  if (nfq_bind_pf(h, AF_INET) < 0)
    {
      fprintf(stderr, "No nfqueue bind, aborting.n");
      die();
    }

  if (!(qh = nfq_create_queue(h, 0, &cb, handle)))
    {
      fprintf(stderr, "No nfqueue queue, aborting.n");
      die();
    }

  if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0)
    {
      fprintf(stderr, "nfqueue mode setup failed, aborting.n");
      die();
    }
   return 0;
}



gint parse_config(struct handle_struct *handle){
	
	GKeyFile *gkf;
	guint64 tmp;
	
//	handle->current_csv = g_string_new("");
    handle->last_sec = 0;
    handle->timeout = 60;
	
	gkf = g_key_file_new();
	
	if (TRUE == g_key_file_load_from_file(gkf,"./default.conf",0,NULL)){
	//if (TRUE == g_key_file_load_from_file(gkf,"/home/jeanm/workspace/jeanm/webfw/src/default.conf",0,NULL)){
		if (TRUE == g_key_file_has_key(gkf,"website","url",NULL)){
			handle->url = g_string_new(g_key_file_get_value(gkf,"website","url",NULL));
			if (0 != handle->url->len)
			  g_message("%s\n",handle->url->str);
		}
		if (TRUE == g_key_file_has_key(gkf,"general","timeout",NULL)){
			tmp = g_ascii_strtoull(g_key_file_get_value(gkf,"general","timeout",NULL),NULL,0);
			if (tmp > MINIMUM_TIMEOUT) handle->timeout = tmp;
		}
	} else {
		g_error("Foxtrot Uniform Charly Kilo");
		syslog(LOG_WARNING, "Foxtrot Uniform Charly Kilo");
	}
	return 0;
}

/**************************************************************************
    Function: Print Usage
 
    Description:
        Output the command-line options for this daemon.
 
    Params:
        @argc - Standard argument count
        @argv - Standard argument array
 
    Returns:
        returns void always
**************************************************************************/
void PrintUsage(int argc, char *argv[]) {
    if (argc >=1) {
        printf("Usage: %s -h -n\n", argv[0]);
        printf("  Options:\n");
        printf("      -n\tDon't fork off as a daemon\n");
        printf("      -h\tShow this help screen\n");
        printf("\n");
    }
}
 
/**************************************************************************
    Function: signal_handler
 
    Description:
        This function handles select signals that the daemon may
        receive.  This gives the daemon a chance to properly shut
        down in emergency situations.  This function is installed
        as a signal handler in the 'main()' function.
 
    Params:
        @sig - The signal received
 
    Returns:
        returns void always
**************************************************************************/
void signal_handler(int sig) {
 
    switch(sig) {
        case SIGHUP:
            syslog(LOG_WARNING, "Received SIGHUP signal.");
            die();
            break;
        case SIGTERM:
            syslog(LOG_WARNING, "Received SIGTERM signal.");
            die();
            break;
        default:
            syslog(LOG_WARNING, "Unhandled signal %s", strsignal(sig));
            break;
    }
}
 
/**************************************************************************
    Function: main
 
    Description:
        The c standard 'main' entry point function.
 
    Params:
        @argc - count of command line arguments given on command line
        @argv - array of arguments given on command line
 
    Returns:
        returns integer which is passed back to the parent process
**************************************************************************/
int main(int argc, char *argv[]) {
 
  int daemonize = 1;

  gint fd;
  gint len;
  FILE *pidfd;
  struct handle_struct handle;

 
    // Setup signal handling before we start
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
 
    int c;
    while( (c = getopt(argc, argv, "nh|help")) != -1) {
        switch(c){
            case 'h':
                PrintUsage(argc, argv);
                exit(0);
                break;
            case 'n':
                daemonize = 0;
                break;
            default:
                PrintUsage(argc, argv);
                exit(0);
                break;
        }
    }
 
    syslog(LOG_INFO, "%s daemon starting up", DAEMON_NAME);
 
    // Setup syslog logging - see SETLOGMASK(3)
#if defined(DEBUG)
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
#else
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(DAEMON_NAME, LOG_CONS, LOG_USER);
#endif
 
    /* Our process ID and Session ID */
    pid_t pid, sid;
 
    if (daemonize) { 
        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
           exit(EXIT_SUCCESS);
        }
 
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
        }
 
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
        }
 
        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
	    /* write PID_FILE if not existend */
        if (NULL == (pidfd = fopen(PID_FILE,"r"))) {
          pidfd = fopen(PID_FILE,"w+");
          assert(pidfd);
          fprintf(pidfd,"%d",getpid());
          fclose(pidfd);
	    } else {
			fclose(pidfd);
            syslog(LOG_INFO, "exiting, another %s daemon is already running", DAEMON_NAME);
			exit(EXIT_FAILURE);
		}

        syslog(LOG_INFO, "%s daemon wrote PID %d", DAEMON_NAME,getpid());

        /* Change the file mode mask */
        umask(0);
 
    }
 
    //****************************************************
  parse_config(&handle);

  listman_init(&handle);
  init_nfq(&handle);

  fd = nfq_fd (h);

  while (1)
    {
      if ((len = recv(fd, bfr, sizeof (bfr), 0)) < 0)
    	  continue;
      nfq_handle_packet(h, (char *) bfr, len);
    }

    //****************************************************
 
    syslog(LOG_INFO, "%s daemon exiting", DAEMON_NAME);
 
    //****************************************************
     deinit_nfq();
    //****************************************************
 
    remove(PID_FILE);
    exit(0);
}
