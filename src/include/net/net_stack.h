#pragma once

#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/dns.h>

namespace net {
	struct net_stack_t {
		net::EtherFrameProvider* ether;
		net::AddressResolutionProtocol* arp;
		net::Ipv4Provider* ipv4;
		net::IcmpProvider* icmp;
		net::UdpProvider* udp;
		net::TcpProvider* tcp;
		net::DomainNameServiceProvider* dns;
	};
}