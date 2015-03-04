/*
 * MIT/X Consortium License
 *
 * Copyright 2014-2015, Truveris Inc. All Rights Reserved.
 * © 2005-2013 Anselm R Garbe <anselm@garbe.us>
 * © 2008-2009 Jeroen Schot <schot@a-eskwadraat.nl>
 * © 2007-2009 Kris Maglione <maglione.k@gmail.com>
 * © 2005 Nico Golde <nico at ngolde dot de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <sys/socket.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <err.h>

#include "dial.h"
#include "lmytfy.h"

int
dial(char *host, char *port) {
	static struct addrinfo hints;
	int srv = -1;
	struct addrinfo *res, *r;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(host, port, &hints, &res) != 0) {
		err(1, "error: cannot resolve hostname '%s':", host);
	}

	for (r = res; r; r = r->ai_next) {
		if ((srv = socket(r->ai_family, r->ai_socktype,
		    r->ai_protocol)) == -1)
			continue;
		if (connect(srv, r->ai_addr, r->ai_addrlen) == 0)
			break;
		close(srv);
	}

	freeaddrinfo(res);

	if (!r || srv == -1) {
		err(1, "error: cannot connect to host '%s'", host);
	}

	/* Do not close this socket if we exec(). */
	if (fcntl(srv, F_SETFD, 0) == -1) {
		err(1, "failed to unset close-on-exec flag");
	}

	return (srv);
}
