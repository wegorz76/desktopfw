What is fwdaemon:
-----------------
fwdaemon is a linux firewall runtime backend. Any client application
can communicate to the fwdaemon and decide which network network flow 
can be accepted or which should be dropped. 

How to run:
-----------
To compile and run fwdaemon you need:
1. NFQUEUE target compiled in your kernel 
2. libraries from http://netfilter.org/
   - libnfnetlink (I use 0.0.25)
   - libnetfilter_queue (I use 0.0.13)
3. glib 2.x

If you have above software installed try to 'make'.

fwdaemon must be run from root account. You'll also need configuration
files (see desktopfw directory in a package, copy this directory to /etc,
you should have 3 files in /etc/desktopfw: apps, config, rules)

As it is in development phase run a fwdeamon from terminal and see what
it logs. 

How it works:
-------------
fwdaemon connects to NFQUEUE and captures incoming and outgoing packets.
When new tcp connection occures fwdaemon scans /proc directory to find out
which application is source/target and decide (using rules) what to do. 
If no rule is matched, connection is queued and waits for user interaction.
User connected to fwdaemon can see what rules are already exists, what
packets are queued and waits for user interaction. 

How to use:
-----------
You can telnet at localhost, port 32123. Available commands you'll get
after LIST command. More information about commands - see protocol_commands.txt
in the package.

Iptables part:
--------------
You need a rules in iptables INPUT/OUTPUT chains. See scripts/00_only_tcp.sh.

What is implemented:
--------------------
Currently I tested tcp protocol. I'm sure there's many bugs in it but
publishing a working code will increase development.

What's the plan?
----------------
I always wanted to have a linux runtime firewall. At this moment I found
only one method how to manage a packets in realtime. This is NFQUEUE
target which allows a user to make a packet decision in userspace. 
I'm writing fwdaemon as a separate application. If you want to write 
a gui application you're welcome. 

TODO:
-----
A lot of...

HOWTO:
------
1. You need to put tcp packets in NFQUEUE, use ./scripts/00_only_tcp.sh
   
2. Start the daemon in one terminal and open another terminal and connect
   to it (use 'rlwrap telnet localhost 32123' for ex).

3. Open your firefox or mozilla or whatever and try to open a web page.

4. There's no rule so fwdaemon will queue a connection and create
   a query entry which require user interaction.
   try 'QUERY LIST'
   You'll see:
   Q[1] IFACE[eth0] DIR[outgoing] SRCIP[192.168.100.100] DSTIP[66.35.250.168] SPORT[58467] DPORT[80] APP[/usr/lib/iceweasel/firefox-bin] INSTIME[1210193183]

   This means an application firefox-bin is trying to connect to some host
   on port 80. As is a http session many other tcp flows will be establish
   according to this session.   

5. To allow application to go out you'll have two possibilities:
   a) create a rule based on query 
   b) create a fully specified rule

   ad a) you can use syntax
         QUERY ADD PERM qnumber [ANYWHERE]
         QUERY ADD SESS qnumber [ANYWHERE]
         so according to query above you can build a permanent or session rule
	 for ex:
	 -------
	 QUERY ADD PERM 1 ANYWHERE 
	 will create new permanent entry in rules based on query number 1
	 rules will be saved on disk
	 -------
	 QUERY ADD SESS 1 ANYWHERE
	 the same as above, but no rules are saved

   ad b) you can build a full rule yourself using syntax         
         RULES ADD SESS iface dir proto srchost srcport dsthost dstport action application
         RULES ADD PERM iface dir proto srchost srcport dsthost dstport action application
	 for above ex:
	 RULES ADD SESS eth0 outgoing tcp 0/0 0 0/0 80 ACCEPT /usr/lib/iceweasel/firefox-bin
	    
6. Session rules are checked first, but they are volatile (after 60 sec of
   inactivity they are removed from rules)
	 
7. When you will disconnect from fwdaemon (CLOSE command) fwdaemon will accept/drop 
   connections based only on rules it has in its memory.

8. As all tcp connections are queued in NFQUEUE, only fwdaemon can make accept/drop decision.
   When you'll stop fwdaemon you need to remove NFQUEUE iptables rules, otherwise all tcp flow
   will be will be dropped.

