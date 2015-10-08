/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwapps.c
*/

#include <fwcommon.h>
#include <fwpacket.h>
#include <fwcomm.h>

#define BUFSIZE    1024
#define MAXSPLIT   16

#if !defined (DEFAUTH) 
#define DEFAULT_AUTHENTICATED TRUE
#endif

#define EMPTY_COMMAND "Empty command (try LIST)\n"
#define END_STRING    "\n"
/* ---------------------------------------------------------------------------- */
gint               csock;
gint               state       = TRUE;
struct sockaddr_in serv_addr;
gboolean           isauth      = FALSE;

/* ---------------------------------------------------------------------------- */
gpointer          _fwcomm_thread(gpointer data) {
	gint       lsock, sport, bread, bwrite;
	gchar      buf[BUFSIZE+1];
	gchar    **strv;
	gchar     *retstr;

	FW_DEBUG_INFO3();

	//Don't crash a daemon if write failed
	signal(SIGPIPE, SIG_IGN);

	sport = fwconfig_get_fw_port();

	FW_GLOBAL_LOCK();
	if ( (lsock = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
		fw_error("Can't create a socket [%s]\n", g_strerror(errno));
	}
	int opt = 1;
	if ( setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ) {
		fw_error("Can't set setsockopt(SO_REUSEADDR) [%s]\n", g_strerror(errno));
	}
	FW_GLOBAL_UNLOCK();

	bzero((gchar *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr( fwconfig_get_fw_host() );
	serv_addr.sin_port        = htons(sport);

	FW_GLOBAL_LOCK();
	if ( bind(lsock, (struct sockaddr *) &serv_addr,  
			  sizeof(struct sockaddr_in)) == -1) {
		fw_error("Can't bind a socket [%s]\n", g_strerror(errno));
	}

	if ( listen(lsock, 1) < 0 ) {
		fw_error("Can't listen on the socket [%s]\n", g_strerror(errno));
	}
	FW_GLOBAL_UNLOCK();

	while (1) {
		fprintf(stderr, "fwdaemon - listening for a new connection\n\n");
		FW_GLOBAL_LOCK();
		if ( (csock = accept(lsock, NULL, NULL)) < 0 ) {
			fw_error("Can't accept a connection  [%s]\n", g_strerror(errno));
		}
		isauth = DEFAULT_AUTHENTICATED;
		FW_GLOBAL_UNLOCK();

		fprintf(stderr, "Socket: %d\n", csock);

		bzero(buf, BUFSIZE);
		strcpy(buf, "This is FWDAEMON, hello!\n\n");
		fprintf(stderr, "BUF: [%s]\n", buf);

		if ( (bwrite = write(csock, buf, sizeof(buf)-1)) < 0 ) {
			fw_warning("Error during write [%s]\n", g_strerror(errno));
			close(csock);
			break;
		}

		for(;;) {
			buf[0] = '\0';
			bread = read(csock, buf, BUFSIZE);
			if (bread <= 0) {
				fprintf(stderr, "Unexpected close\n");
				close(csock);
			}

			buf[bread] = '\0';

			//remove trailing \n and \r (for ex for telnet connection)
			if ( bread >=2 ) {
				if ( buf[bread-1] == '\n' || buf[bread-1] == '\r' ) buf[bread-1] = '\0';
				if ( buf[bread-2] == '\n' || buf[bread-2] == '\r' ) buf[bread-2] = '\0';
				bread = strlen(buf);
			}

			//if bytes read == 0 (after \n and \r removal) there's empty command.
			if (!bread) {
				if ( (bwrite = write(csock, EMPTY_COMMAND, strlen(EMPTY_COMMAND))) < 0 ) {
					fw_warning("Error during write [%s]\n", g_strerror(errno));
					close(csock);
					break;
				}
				continue;
			}

			// strv will need to bo free
			strv = g_strsplit(buf, " ", MAXSPLIT);
			
			if ( isauth == FALSE && strcmp(strv[0], "AUTH") ) {
				retstr = g_strdup("ERROR: Not authenticated\n");
			} else {
				// callback function returns allocated string
				retstr = fw_command_call(strv);
			}

			if ( (bwrite = write(csock, retstr, strlen(retstr))) < 0 ) {
				fw_warning("Error during write [%s]\n", g_strerror(errno));
				close(csock);
				break;
			}
			g_free(retstr);
			fflush(stdout);

			if ( !strcmp(strv[0], "EXIT") ) {
				write(csock, END_STRING, strlen(END_STRING) );
				goto end;
			}

			if ( !strcmp(strv[0], "CLOSE") ) {
				g_strfreev(strv);
				close(csock);
				break;
			}

			g_strfreev(strv);
			write(csock, END_STRING, strlen(END_STRING) );
			//sleep(1);
		}
	}

	sleep(1000);

end:
	state = FALSE;
	g_strfreev(strv);
	return NULL;
}

/* Create communication thread */
gboolean          fwcomm_thread_create() {
  GThread *cthread;

  FW_DEBUG_INFO3();

  cthread = g_thread_create( _fwcomm_thread, NULL, FALSE, NULL );

  return TRUE;
}

/* ---------------------------------------------------------------------------- */
inline gboolean   fwcomm_thread_is_running() {
  return state;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
gchar            *fwcomm_cb_auth(gchar **strv) {
  FW_DEBUG_INFO3();

  if ( g_strv_length(strv) <= 2 ) {
    return g_strdup("usage: AUTH username password\n");
  }  

  if ( !strcmp(strv[1], fwconfig_get_fw_username()) &&
       !strcmp(strv[2], fwconfig_get_fw_password()) ) {
    isauth = TRUE;
    return g_strdup("AUTH OK\n");    
  }

  return g_strdup("AUTH ERROR\n");
}

/* ---------------------------------------------------------------------------- */
void              fwcomm_register_cbs() {
  FW_DEBUG_INFO3();

  fw_command_add_callback("AUTH",      fwcomm_cb_auth);
}
