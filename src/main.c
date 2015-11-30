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

#include <sys/select.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "dial.h"
#include "irc.h"
#include "log.h"
#include "parse.h"

char *host = "localhost";
char *port = "6667";
char *nick = "lmytfy";
char *name = "let me ygor that for you";
FILE *srv;

const char *channels[] = {
	"#dev",
	"#ops",
	"#truveris",
	NULL
};

static time_t trespond;

#ifndef REGRESS
int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	int n;
	fd_set rd;
	struct timeval tv;
	static char bufin[4096];

	init_regexes();

	if ((srv = fdopen(dial(host, port), "r+")) == NULL)
		err(1, "error: fdopen failed: ");

	irc_startup();

	/* Disable buffering */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(srv, NULL, _IONBF, 0);

	for (;;) {
		FD_ZERO(&rd);
		FD_SET(0, &rd);
		FD_SET(fileno(srv), &rd);
		tv.tv_sec = 120;
		tv.tv_usec = 0;
		n = select(fileno(srv) + 1, &rd, 0, 0, &tv);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			err(1, "error: select() failed: ");
		} else if (n == 0) {
			if (time(NULL) - trespond >= 300) {
				errx(1, "error: parse timeout");
			}
			irc_printf("PING %s", host);
			continue;
		}

		if (FD_ISSET(fileno(srv), &rd)) {
			if (fgets(bufin, sizeof(bufin), srv) == NULL) {
				err(1, "error: server closed connection");
			}
			irc_parse(bufin);
			trespond = time(NULL);
		}
	}

	return (0);
}
#endif // ifndef REGRESS
