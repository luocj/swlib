﻿/*************************************************************************************
*filename:	base_inet_addr.h
*
*Create on: 2012-04
*Author:	zerok
*************************************************************************************/
#ifndef __BASE_INET_ADDR_H
#define __BASE_INET_ADDR_H

#include "base_namespace.h"
#include "base_os.h"
#include "base_def.h"
#include "base_typedef.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

class Inet_Addr
{
public:
	Inet_Addr();
	Inet_Addr(const Inet_Addr& addr);
	Inet_Addr(const string& hostname, uint16_t port);
	Inet_Addr(uint32_t ip, uint16_t port);
	Inet_Addr(const sockaddr_in& addr);

	virtual ~Inet_Addr();

	bool			is_null() const;
	Inet_Addr&		operator=(const Inet_Addr& addr);
	Inet_Addr&		operator=(const sockaddr_in& addr);
	Inet_Addr&		operator=(const string& addr_str);

	void			set_ip(uint32_t ip);
	void			set_ip(const string& hostname);
	void			set_port(uint16_t port);

	uint32_t		get_ip() const;
	uint16_t		get_port() const;
	const sockaddr_in& ipaddr() const;
	sockaddr_in*	get_addr();
	const sockaddr_in*	get_addr() const;
	string			to_string() const;
	string			ip_to_string() const;

	bool			operator==(const Inet_Addr& addr) const;
	bool			operator!=(const Inet_Addr& addr) const;
	bool			operator>(const Inet_Addr& addr) const;
	bool			operator<(const Inet_Addr& addr) const;

private:
	sockaddr_in		addr_;
};
BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/

