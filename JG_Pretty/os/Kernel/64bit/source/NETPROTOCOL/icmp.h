/*
 *  ZeX/OS
 *  Copyright (C) 2008  Tomas 'ZeXx86' Jedrzejek (zexx86@zexos.org)
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

#ifndef _ICMP_H
#define _ICMP_H

#include "ip.h"

#define NET_ICMP_TYPE_PING_REQUEST	0x8
#define NET_ICMP_TYPE_PING_REPLY	0x0
#define NET_ICMP6_TYPE_PING_REQUEST	0x80
#define NET_ICMP6_TYPE_PING_REPLY	0x81

/* ICMP layer structure */
typedef struct proto_icmp_t {
	unsigned char type;
	unsigned char code;
	unsigned short checksum;
	unsigned short ident;
	unsigned short seq;
} proto_icmp_t;

extern unsigned net_proto_icmp_ping (netif_t *netif, net_ipv4 ip);
extern unsigned net_proto_icmp6_ping (netif_t *netif, net_ipv6 ip);

#endif
