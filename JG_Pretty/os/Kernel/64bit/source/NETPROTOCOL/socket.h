/*
 *  ZeX/OS
 *  Copyright (C) 2007  Tomas 'ZeXx86' Jedrzejek (zexx86@zexos.org)
 *  Copyright (C) 2010  Tomas 'ZeXx86' Jedrzejek (zexx86@zexos.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SOCKET_H
#define _SOCKET_H

#include "../types.h"

/* Supported address families. */
#define AF_UNSPEC	0
#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/
#define AF_INET		2	/* Internet IP Protocol 	*/
#define AF_AX25		3	/* Amateur Radio AX.25 		*/
#define AF_IPX		4	/* Novell IPX 			*/
#define AF_APPLETALK	5	/* AppleTalk DDP 		*/
#define AF_NETROM	6	/* Amateur Radio NET/ROM 	*/
#define AF_BRIDGE	7	/* Multiprotocol bridge 	*/
#define AF_ATMPVC	8	/* ATM PVCs			*/
#define AF_X25		9	/* Reserved for X.25 project 	*/
#define AF_INET6	10	/* IP version 6			*/
#define AF_ROSE		11	/* Amateur Radio X.25 PLP	*/
#define AF_DECnet	12	/* Reserved for DECnet project	*/
#define AF_NETBEUI	13	/* Reserved for 802.2LLC project*/
#define AF_SECURITY	14	/* Security callback pseudo AF */
#define AF_KEY		15      /* PF_KEY key management API */
#define AF_NETLINK	16
#define AF_ROUTE	AF_NETLINK /* Alias to emulate 4.4BSD */
#define AF_PACKET	17	/* Packet family		*/
#define AF_ASH		18	/* Ash				*/
#define AF_ECONET	19	/* Acorn Econet			*/
#define AF_ATMSVC	20	/* ATM SVCs			*/
#define AF_SNA		22	/* Linux SNA Project (nutters!) */
#define AF_IRDA		23	/* IRDA sockets			*/
#define AF_PPPOX	24	/* PPPoX sockets		*/
#define AF_WANPIPE	25	/* Wanpipe API Sockets */
#define AF_LLC		26	/* Linux LLC			*/
#define AF_TIPC		30	/* TIPC sockets			*/
#define AF_BLUETOOTH	31	/* Bluetooth sockets 		*/
#define AF_IUCV		32	/* IUCV sockets			*/
#define AF_RXRPC	33	/* RxRPC sockets 		*/
#define AF_MAX		34	/* For now.. */
#define AF_RS232	35	/* Serial socket (NEW!) */

/* Protocol families, same as address families. */
#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_LOCAL	AF_LOCAL
#define PF_INET		AF_INET
#define PF_AX25		AF_AX25
#define PF_IPX		AF_IPX
#define PF_APPLETALK	AF_APPLETALK
#define	PF_NETROM	AF_NETROM
#define PF_BRIDGE	AF_BRIDGE
#define PF_ATMPVC	AF_ATMPVC
#define PF_X25		AF_X25
#define PF_INET6	AF_INET6
#define PF_ROSE		AF_ROSE
#define PF_DECnet	AF_DECnet
#define PF_NETBEUI	AF_NETBEUI
#define PF_SECURITY	AF_SECURITY
#define PF_KEY		AF_KEY
#define PF_NETLINK	AF_NETLINK
#define PF_ROUTE	AF_ROUTE
#define PF_PACKET	AF_PACKET
#define PF_ASH		AF_ASH
#define PF_ECONET	AF_ECONET
#define PF_ATMSVC	AF_ATMSVC
#define PF_SNA		AF_SNA
#define PF_IRDA		AF_IRDA
#define PF_PPPOX	AF_PPPOX
#define PF_WANPIPE	AF_WANPIPE
#define PF_LLC		AF_LLC
#define PF_TIPC		AF_TIPC
#define PF_BLUETOOTH	AF_BLUETOOTH
#define PF_IUCV		AF_IUCV
#define PF_RXRPC	AF_RXRPC
#define PF_MAX		AF_MAX
#define PF_RS232	AF_RS232

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	128

/* Flags we can use with send/ and recv. 
   Added those for 1003.1g not all are supported yet
 */
 
#define MSG_OOB		1
#define MSG_PEEK	2
#define MSG_DONTROUTE	4
#define MSG_TRYHARD     4       /* Synonym for MSG_DONTROUTE for DECnet */
#define MSG_CTRUNC	8
#define MSG_PROBE	0x10	/* Do not send. Only probe path f.e. for MTU */
#define MSG_TRUNC	0x20
#define MSG_DONTWAIT	0x40	/* Nonblocking io		 */
#define MSG_EOR         0x80	/* End of record */
#define MSG_WAITALL	0x100	/* Wait for a full request */
#define MSG_FIN         0x200
#define MSG_SYN		0x400
#define MSG_CONFIRM	0x800	/* Confirm path validity */
#define MSG_RST		0x1000
#define MSG_ERRQUEUE	0x2000	/* Fetch message from error queue */
#define MSG_NOSIGNAL	0x4000	/* Do not generate SIGPIPE */
#define MSG_MORE	0x8000	/* Sender will send more */

#define MSG_EOF         MSG_FIN

#if defined(CONFIG_COMPAT)
#define MSG_CMSG_COMPAT	0x80000000	/* This message needs 32 bit fixups */
#else
#define MSG_CMSG_COMPAT	0		/* We never have 32 bit fixups */
#endif


/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_IP		0
/* #define SOL_ICMP	1	No-no-no! Due to Linux :-) we cannot use SOL_ICMP=1 */
#define SOL_TCP		6
#define SOL_UDP		17
#define SOL_IPV6	41
#define SOL_ICMPV6	58
#define SOL_SCTP	132
#define SOL_UDPLITE	136     /* UDP-Lite (RFC 3828) */
#define SOL_RAW		255
#define SOL_IPX		256
#define SOL_AX25	257
#define SOL_ATALK	258
#define SOL_NETROM	259
#define SOL_ROSE	260
#define SOL_DECNET	261
#define	SOL_X25		262
#define SOL_PACKET	263
#define SOL_ATM		264	/* ATM layer (cell level) */
#define SOL_AAL		265	/* ATM Adaption Layer (packet level) */
#define SOL_IRDA        266
#define SOL_NETBEUI	267
#define SOL_LLC		268
#define SOL_DCCP	269
#define SOL_NETLINK	270
#define SOL_TIPC	271
#define SOL_RXRPC	272

/* IPX options */
#define IPX_TYPE	1

/* Sock types */
#define SOCK_STREAM	1		/* stream (connection) socket	*/
#define SOCK_DGRAM	2		/* datagram (conn.less) socket	*/
#define SOCK_RAW	3		/* raw socket			*/
#define SOCK_RDM	4		/* reliably-delivered message	*/
#define SOCK_SEQPACKET	5		/* sequential packet socket	*/
#define SOCK_PACKET	10		/* linux specified 		*/

#define 	_NETINET_IN_H_
#define 	IPPROTO_IP   	0	// Dummy for IP.
#define 	IPPROTO_ICMP    1	// Control message protocol.
#define 	IPPROTO_IGMP    2	// Group management protocol.
#define 	IPPROTO_TCP     6	// Transmission control protocol.
#define 	IPPROTO_UDP     17 	// User datagram protocol.
#define 	IPPROTO_RAW   	255	// Raw IP packet.
#define 	IPPROTO_MAX   	256     // Maximum protocol identifier.
#define 	IPPORT_RESERVED 1024    // Last reserved port number.
#define 	IPPORT_USERRESERVED   5000 // User reserved port number.
#define 	INADDR_ANY   		0x00000000 // Any IP address.
#define 	INADDR_BROADCAST   	0xffffffff // Broadcast IP address.

#define		FD_SOCK		0x100

typedef unsigned socklen_t;

/* Device structure */
typedef struct socket_context {
	struct socket_context *next, *prev;

	unsigned fd;
	int family;
	int type;
	int protocol;
} socket_t;

typedef struct {
      unsigned short s6_addr[8];  /* IPv6 address */
} in6_addr;

typedef struct {
	unsigned short sa_family;
	char sa_data[14];
} sockaddr;

typedef struct
{
	int sin_family;
	int sin_port;
	unsigned sin_addr;
} sockaddr_in;

typedef struct {
          unsigned char sin6_len;      /* length of this structure */
          unsigned char sin6_family;   /* AF_INET6                 */
          unsigned short sin6_port;     /* Transport layer port #   */
          unsigned sin6_flowinfo; /* IPv6 flow information    */
          unsigned short sin6_addr[8];     /* IPv6 address             */
} sockaddr_in6;

typedef struct {
	  unsigned short sun_family;  /* AF_UNIX */
	  char sun_path[108];
} sockaddr_un;

typedef struct {
	char *h_name;
	char **h_aliases;
	int h_addrtype;
	unsigned h_length;
	char **h_addr_list;
	char *h_addr;
} hostent;

socket_t *socket_getbyfd (int fd);
hostent *gethostbyname (char *hostname);
int socket (int family, int type, int protocol);
int connect (int fd, sockaddr *addr, socklen_t len);
int send (int fd, char *msg, size_t size, int flags);
int recv (int fd, char *msg, size_t size, int flags);
int bind (int fd, sockaddr *addr, socklen_t len);
int sclose (int fd);
int sfcntl (int fd, int cmd, long arg);
int sselect (int readfd, int writefd, int exceptfd);
unsigned int init_socket ();
unsigned inet_addr (const char *src);
#endif
