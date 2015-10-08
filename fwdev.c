/*
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */

/* ---------------------------------------------------------------------------- */
/*! 
	\file fwdev.c
*/

#include <fwcommon.h>

/* ---------------------------------------------------------------------------- */
G_LOCK_DEFINE(FWDevsLock);

static GArray      *FWDevs                = NULL;
static GTimeVal    last_check             = { 0, 0 };
/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
/*! Add device entry to FWDevs GArray
  \param ...
  \return Pointer to FWDevEntry
 */
static FWDevEntry *fwdev_entry_add(gchar *iface,
				   gulong    rbytes,
				   gulong    rpackets,
				   gulong    rerrs,
				   gulong    rdrop,
				   gulong    rfifo,
				   gulong    rframe,
				   gulong    rcompressed,
				   gulong    rmulticast,
				   gulong    tbytes,
				   gulong    tpackets,
				   gulong    terrs,
				   gulong    tdrop,
				   gulong    tfifo,
				   gulong    tframe,
				   gulong    tcompressed,
				   gulong    tmulticast)
{
	FWDevEntry *e;

	FW_DEBUG_INFO3();

	e = g_new(FWDevEntry, 1);

	/* Slot number */
	strncpy(e->iface, iface, 16);
	fw_message3("Iface: %s", e->iface);
	
	/* rbytes, rpackets, rerrs, rdrop, rfifo, rframe, rcompressed, rmulticast */
	e->rbytes      = rbytes;
	e->rpackets    = rpackets;
	e->rerrs       = rerrs;
	e->rdrop       = rdrop;
	e->rfifo       = rfifo;
	e->rframe      = rframe;
	e->rcompressed = rcompressed;
	e->rmulticast  = rmulticast;
	fw_message3("rbytes [%ld], rpackets [%ld], rerrs [%ld], rdrop [%ld]", rbytes, rpackets, rerrs, rdrop);
	fw_message3("rfifo [%ld], rframe [%ld], rcompressed [%ld], rmulticast [%ld]", rfifo, rframe, rcompressed, rmulticast);

	/* tbytes, tpackets, terrs, tdrop, tfifo, tframe, tcompressed, tmulticast */
	e->tbytes      = tbytes;
	e->tpackets    = tpackets;
	e->terrs       = terrs;
	e->tdrop       = tdrop;
	e->tfifo       = tfifo;
	e->tframe      = tframe;
	e->tcompressed = tcompressed;
	e->tmulticast  = tmulticast;
	fw_message3("tbytes [%ld], tpackets [%ld], terrs [%ld], tdrop [%ld]", tbytes, tpackets, terrs, tdrop);
	fw_message3("tfifo [%ld], tframe [%ld], tcompressed [%ld], tmulticast [%ld]", tfifo, tframe, tcompressed, tmulticast);
	
	g_array_append_val(FWDevs, e);

	return e;
}

/* ---------------------------------------------------------------------------- */
static gboolean   fwdev_entry_free(gint n)
{
	FWDevEntry *e;

	FW_DEBUG_INFO3();

	if ( (e = g_array_index(FWDevs, FWDevEntry*, n)) == NULL ) {
		fw_warning("FWDevEntry entry == NULL!");
		return FALSE;
	}
	FW_FREEN(e);
	g_array_remove_index(FWDevs, n);

	return TRUE;
}

/* ---------------------------------------------------------------------------- */
void              fwdev_free(void)
{
	gint i;
	gint len;

	FW_DEBUG_INFO3();

	len = FWDevs->len;
	for (i = 0; i < len; i++) {
		fwdev_entry_free(0);
	}
	g_array_free(FWDevs, TRUE);
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
gint              fwdev_read(void)
{
	GTimeVal tv;
	FILE  *in;
	gchar  buf[BUFSIZE], iface[IFNAMSIZ];
	gint num;
	gulong rbytes, rpackets, rerrs, rdrop, rfifo, rframe, rcompressed, rmulticast;
	gulong tbytes, tpackets, terrs, tdrop, tfifo, tframe, tcompressed, tmulticast;
	struct nlif_handle *h;
	gint scanned, i, j;

	FW_DEBUG_INFO3();

	//read devices not often than FWDEV_CHECKTIME_INTERVAL seconds
	g_get_current_time(&tv);
	if ( (tv.tv_sec - last_check.tv_sec) < FWDEV_CHECKTIME_INTERVAL)
		return 0;
	last_check.tv_sec  = tv.tv_sec;
	last_check.tv_usec = tv.tv_usec;

	G_LOCK(FWDevsLock); 

	if (FWDevs) {
		fw_message3("Freeing current array of devices");
		g_message("Freeing current array of devices");
		fwdev_free();
	}
	FWDevs = g_array_new(FALSE, FALSE, sizeof(FWDevEntry *));
   
	if ( (in = fopen(FWNETDEV, "rb")) == NULL )
		g_error("Can't open [%s] file. Exiting.\nError: %s", FWNETDEV, g_strerror(errno));

	//skip two first lines
	fgets(buf, BUFSIZE-1, in);
	fgets(buf, BUFSIZE-1, in);
	
	while (!feof(in)) {
		fgets(buf, BUFSIZE-1, in);
		if (feof(in)) 
			continue;
		
		g_strchug(buf);
		g_strchomp(buf);
		fw_message3("input line: [%s]", buf);
		
		num = sscanf(buf,
					 "%[a-z0-9.]:%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
					 iface, 
					 &rbytes, &rpackets, &rerrs, &rdrop, &rfifo, &rframe, &rcompressed, &rmulticast,
					 &tbytes, &tpackets, &terrs, &tdrop, &tfifo, &tframe, &tcompressed, &tmulticast);

		if (num != 17) 
			continue;

		fwdev_entry_add(
			iface, 
			rbytes, rpackets, rerrs, rdrop, rfifo, rframe, rcompressed, rmulticast,
			tbytes, tpackets, terrs, tdrop, tfifo, tframe, tcompressed, tmulticast);
	}
	fclose(in);

	/* netfilter interface idx differs, need to scan nlif */
	h = nlif_open();
	if (h == NULL) 
		g_assert("Can't open nlif");
	nlif_query(h);

	num     = FWDevs->len;
	scanned = 0;
	i       = 0;
	while (1) {
		if (nlif_index2name(h, i, iface) == -1) {
			i++;
			continue;
		}
		//printf("index (%d) is %s\n", i, iface);
		for (j = 0; j < num; j++) {
			FWDevEntry *e;
			e   = g_array_index(FWDevs, FWDevEntry*, j);
			if (!strcmp(e->iface, iface))
			{
				e->nlifidx = i-1;
				break;
			}
		}
		i++;
		if (scanned++ == num) break;
	}
	nlif_close(h);

	G_UNLOCK(FWDevsLock);

	return 0;
}

/* ---------------------------------------------------------------------------- */
/* Force devices reread
 */
gint              fwdev_force_read(void) {
	last_check.tv_sec  = 0;
	last_check.tv_usec = 0;
	return fwdev_read();
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
gchar            *_fwdev_entry_stats(gint n) {
	FWDevEntry *e;
	GString    *s;

	FW_DEBUG_INFO3();

	s = g_string_new("");

	if ( (e = g_array_index(FWDevs, FWDevEntry*, n)) == NULL ) {
		g_string_free(s, TRUE);
		return NULL;
	}

	g_string_append_printf(s, "DevIndex: [%d]\n",     n);
	g_string_append_printf(s, "Interface: [%s]\n",    e->iface);
	g_string_append_printf(s, "NLIFIndex: [%d]\n",    e->nlifidx);
	
	/* Receive */
	g_string_append_printf(s, "rbytes: [%ld]\n",      e->rbytes);
	g_string_append_printf(s, "rpackets: [%ld]\n",    e->rpackets);
	g_string_append_printf(s, "rerrs: [%ld]\n",       e->rerrs);
	g_string_append_printf(s, "rdrop: [%ld]\n",       e->rdrop);
	g_string_append_printf(s, "rfifo: [%ld]\n",       e->rfifo);
	g_string_append_printf(s, "rframe: [%ld]\n",      e->rframe);
	g_string_append_printf(s, "rcompressed: [%ld]\n", e->rcompressed);
	g_string_append_printf(s, "rmulticast: [%ld]\n",  e->rmulticast);

	/* Transmit */
	g_string_append_printf(s, "tbytes: [%ld]\n",      e->tbytes);
	g_string_append_printf(s, "tpackets: [%ld]\n",    e->tpackets);
	g_string_append_printf(s, "terrs: [%ld]\n",       e->terrs);
	g_string_append_printf(s, "tdrop: [%ld]\n",       e->tdrop);
	g_string_append_printf(s, "tfifo: [%ld]\n",       e->tfifo);
	g_string_append_printf(s, "tframe: [%ld]\n",      e->tframe);
	g_string_append_printf(s, "tcompressed: [%ld]\n", e->tcompressed);
	g_string_append_printf(s, "tmulticast: [%ld]\n",  e->tmulticast);
	g_string_append(s, "--------------------------------------\n");
	
	return g_string_free( s, FALSE );
}

/* ---------------------------------------------------------------------------- */
FWDevEntry       *fwdev_entry_by_idx(gint idx) {
	FWDevEntry *e;

	FW_DEBUG_INFO3();

	if (idx >= 0 && idx < FWDevs->len) 
		e = g_array_index(FWDevs, FWDevEntry*, idx);
	else 
		e = NULL;
  
	return e;
}

/* ---------------------------------------------------------------------------- */
gchar            *fwdev_string_by_idx(gint idx)
{
	FWDevEntry *e;
	gint l;

	FW_DEBUG_INFO3();

	l = FWDevs->len;
	if (idx < 0 || idx >= l) {
		g_warning("Idx out of range <0,%d>", l-1);
		return NULL;
	}
	e = g_array_index(FWDevs, FWDevEntry*, idx);
	return e->iface;
}

/* ---------------------------------------------------------------------------- */
gchar            *fwdev_string_by_nf_idx(gint nlifidx)
{
	FWDevEntry *e;
	gint l, i;
	
	FW_DEBUG_INFO3();
	l = FWDevs->len;
	for (i = 0; i < l; i++) {
		e = g_array_index(FWDevs, FWDevEntry*, i);
		if (e->nlifidx == nlifidx) {
			//printf("nfidx = %s %d %d\n", e->iface, e->nlifidx, i);
			break;
		}
	}
	return e->iface;
}

/* ---------------------------------------------------------------------------- */
static void       fwdev_entry_print(gint n)
{
	gchar *s;

	FW_DEBUG_INFO3();

	s = _fwdev_entry_stats(n);
	if ( s == NULL ) {
		printf("fwdev_entry_print: no such device [%d]\n", n);
		return;
	}

	printf("%s", s);
	g_free(s);
}

/* ---------------------------------------------------------------------------- */
gint              fwdev_print(void)
{
	gint i, l;

	FW_DEBUG_INFO3();

	l = FWDevs->len;
	for (i = 0; i < l; i++) 
		fwdev_entry_print(i);

	return l;
}

/* ---------------------------------------------------------------------------- */
gint              fwdev_find_ifaceidx(gchar *iface)
{
	FWDevEntry *e;
	int i, l;

	FW_DEBUG_INFO3();

	l = FWDevs->len;
	for (i = 0; i < l; i++) {
		e   = g_array_index(FWDevs, FWDevEntry*, i);
		if ( !strcmp(e->iface, iface) ) {
			return i;
		}
	}

	return -1;
}

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------- */
gchar            *_fwdev_get_cbs() {
	return g_strconcat( "Available DEV commands:\n" 
						" DEV LIST\n",
						" DEV STATS\n",
						NULL );
}

gchar            *fwdev_cb_dev(gchar **strv) {
	gchar *retstr;

	FW_DEBUG_INFO3();

	if (g_strv_length(strv) == 1) {
		/* Show available APP commands */
		return _fwdev_get_cbs();
	} 

	if ( !strcmp(strv[1], "LIST") ) 
		retstr = fwdev_cb_dev_list(strv);
	else if ( !strcmp(strv[1], "STATS") ) 
		retstr = fwdev_cb_dev_stats(strv);
	else 
		retstr = _fwdev_get_cbs();

	return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for DEV LIST command
*/
gchar            *fwdev_cb_dev_list(gchar **strv) {
	FWDevEntry *e;
	GString    *s;
	gchar      *retstr;
	gint        i, l;

	FW_DEBUG_INFO3();

	fwdev_read();
	s = g_string_new("");

	G_LOCK(FWDevsLock);

	l = FWDevs->len;
	for (i = 0; i < l; i++) {
		e   = g_array_index(FWDevs, FWDevEntry*, i);
		g_string_append_printf( s, "[%d] %s nlifidx=%d\n", i, e->iface, e->nlifidx );
	}

	G_UNLOCK(FWDevsLock);

	retstr = g_string_free( s, FALSE );

	return retstr;
}

/* ---------------------------------------------------------------------------- */
/*! Callback for DEV STATS device command
*/
gchar            *fwdev_cb_dev_stats(gchar **strv) {
	FWDevEntry *e;
	gchar      *retstr;
	gint        n;

	FW_DEBUG_INFO3();

	if ( g_strv_length(strv) != 3 ) {
		return g_strdup("usage: DEV STATS interface\n");
	}

	//Reread device statistics
	fwdev_read();

	G_LOCK(FWDevsLock);

	n = fwdev_find_ifaceidx(strv[2]);
	if (n < 0) {
		G_UNLOCK(FWDevsLock);
		return g_strdup_printf("ERROR: DEV STATS interface [can't find iface=%s]\n", strv[2]);
	}
	e = g_array_index(FWDevs, FWDevEntry*, n);
	retstr = _fwdev_entry_stats(n);

	G_UNLOCK(FWDevsLock);

	return retstr;
}

/* ---------------------------------------------------------------------------- */
void              fwdev_register_cbs() {
	FW_DEBUG_INFO3();

	fw_command_add_callback("DEV",      fwdev_cb_dev);
}

