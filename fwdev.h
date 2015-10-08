/* 
 * Copyright (C) Zbigniew Kempczyński wegorz76@gmail.com
 */
/* ---------------------------------------------------------------------------- */
#if !defined __FWDEV_H
#define __FWDEV_H

#include <fwcommon.h>
#include <libnfnetlink/libnfnetlink.h>
/* ---------------------------------------------------------------------------- */
#define BUFSIZE 1024

#define FWNETDEV     "/proc/net/dev"
#define FWDEV_CHECKTIME_INTERVAL 2
/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
typedef struct {
	gchar     iface[IFNAMSIZ];
	gint      nlifidx;            //netlink interface index
	gulong    rbytes;
	gulong    rpackets;
	gulong    rerrs;
	gulong    rdrop;
	gulong    rfifo;
	gulong    rframe;
	gulong    rcompressed;
	gulong    rmulticast;
	gulong    tbytes;
	gulong    tpackets;
	gulong    terrs;
	gulong    tdrop;
	gulong    tfifo;
	gulong    tframe;
	gulong    tcompressed;
	gulong    tmulticast;
} FWDevEntry;

/* ---------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------- */
void              fwdev_free(void);
gint              fwdev_read(void);
gint              fwdev_force_read(void);

FWDevEntry       *fwdev_entry_by_idx(gint idx);
gchar            *fwdev_string_by_idx(gint idx);
gchar            *fwdev_string_by_nf_idx(gint nlifidx);
gint              fwdev_print(void);

/* Find an iface index in FWDevs array */
gint              fwdev_find_ifaceidx(gchar *iface);

/* ---------------------------------------------------------------------------- */
gchar            *fwdev_cb_dev(gchar **strv);
gchar            *fwdev_cb_dev_list(gchar **strv);
gchar            *fwdev_cb_dev_stats(gchar **strv);
void              fwdev_register_cbs();
#endif 
