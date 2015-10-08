		CC	= gcc -Wall -I.
		CFLAGS	= `pkg-config --cflags glib-2.0` -DDEBUG=1 -DFWDEBUG=1 -DFWFUNCSDEBUG=1 -ggdb
		LDFLAGS	= `pkg-config --libs glib-2.0` `pkg-config --libs gthread-2.0` -lnfnetlink -lnetfilter_queue 

		DEFAUTH = -DTRUE
		OBJS	= fwcommon.o fwutils.o fwpacket.o fwconfig.o fwcomm.o fwapps.o fwrules.o fwproc.o fwdev.o fwquery.o fwnfq.o fwnfq_ip.o fwnfq_icmp.o fwnfq_tcp.o fwnfq_udp.o
		DEPS	= fwcommon.h fwutils.h fwmacros.h

all:		fwdaemon test_fwconfig test_fwapps test_fwrules test_fwproc test_fwdev

fwdaemon:	fwdaemon.o $(OBJS)
		$(CC) -o fwdaemon fwdaemon.o $(OBJS) $(LDFLAGS)

test_fwconfig:	test_fwconfig.o fwcommon.o fwconfig.o
		$(CC) -o test_fwconfig test_fwconfig.o fwcommon.o fwconfig.o fwpacket.o $(LDFLAGS)
test_fwapps:	test_fwapps.o fwcommon.o fwapps.o
		$(CC) -o test_fwapps test_fwapps.o fwcommon.o fwapps.o fwpacket.o $(LDFLAGS) 

test_fwrules:	test_fwrules.o fwcommon.o fwrules.o
		$(CC) -o test_fwrules test_fwrules.o fwconfig.o fwcommon.o fwrules.o fwpacket.o fwdev.o $(LDFLAGS) 

test_fwproc:	test_fwproc.o $(OBJS)
		$(CC) -o test_fwproc test_fwproc.o $(OBJS) $(LDFLAGS) 

test_fwdev:	test_fwdev.o $(OBJS)
		$(CC) -o test_fwdev test_fwdev.o $(OBJS) $(LDFLAGS) 

fwdaemon.o:	fwdaemon.c $(DEPS)
		$(CC) -c fwdaemon.c $(CFLAGS)

fwcommon.o:	fwcommon.c $(DEPS)
		$(CC) -c fwcommon.c $(CFLAGS) 

fwutils.o:	fwutils.c $(DEPS)
		$(CC) -c fwutils.c $(CFLAGS) 

fwpacket.o:	fwpacket.c $(DEPS)
		$(CC) -c fwpacket.c $(CFLAGS) 

fwconfig.o:	fwconfig.c fwconfig.h $(DEPS)
		$(CC) -c fwconfig.c $(CFLAGS)

fwcomm.o:	fwcomm.c fwcomm.h $(DEPS)
		$(CC) -c fwcomm.c $(CFLAGS)

fwapps.o:	fwapps.c fwapps.h $(DEPS)
		$(CC) -c fwapps.c $(CFLAGS)

fwrules.o:	fwrules.c fwrules.h $(DEPS)
		$(CC) -c fwrules.c $(CFLAGS)

fwproc.o:	fwproc.c fwproc.h $(DEPS)
		$(CC) -c fwproc.c $(CFLAGS)

fwdev.o:	fwdev.c fwdev.h $(DEPS)
		$(CC) -c fwdev.c $(CFLAGS)

fwquery.o:	fwquery.c fwquery.h $(DEPS)
		$(CC) -c fwquery.c $(CFLAGS)

fwnfq.o:	fwnfq.c fwnfq.h $(DEPS)
		$(CC) -c fwnfq.c $(CFLAGS)

fwnfq_ip.o:	fwnfq.c fwnfq.h fwnfq_ip.c fwnfq_ip.h $(DEPS)
		$(CC) -c fwnfq_ip.c $(CFLAGS)

fwnfq_icmp.o:	fwnfq.c fwnfq.h fwnfq_icmp.c fwnfq_icmp.h $(DEPS)
		$(CC) -c fwnfq_icmp.c $(CFLAGS)

fwnfq_tcp.o:	fwnfq.c fwnfq.h fwnfq_tcp.c fwnfq_tcp.h $(DEPS)
		$(CC) -c fwnfq_tcp.c $(CFLAGS)

fwnfq_udp.o:	fwnfq.c fwnfq.h fwnfq_udp.c fwnfq_udp.h $(DEPS)
		$(CC) -c fwnfq_udp.c $(CFLAGS)

test_fwconfig.o: test_fwconfig.c $(DEPS)
		$(CC) -c test_fwconfig.c $(CFLAGS)

test_fwproc.o:  test_fwproc.c $(DEPS)
		$(CC) -c test_fwproc.c $(CFLAGS)
clean:;
		rm -f *.o *~ fwdaemon test_fwconfig test_fwproc test_fwdev test_fwapps test_fwrules
