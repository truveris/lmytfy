/*
 * MIT/X Consortium License
 *
 * Copyright 2014, Truveris Inc. All Rights Reserved.
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

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <regex.h>

#include "util.h"


#define PROGRAM "lmytfy"


regex_t ygor_preg, lmytfy_preg;
static char *progname;
static char *host = "localhost";
static char *port = "6667";
static char *name = "let me ygor that for you";
static char *nick = "lmytfy";
static char bufin[4096];
char bufout[4096];
static time_t trespond;
static FILE *srv;


static void
pout(char *channel, char *fmt, ...) {
	static char timestr[80];
	time_t t;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	t = time(NULL);
	strftime(timestr, sizeof timestr, "%Y-%m-%d %R", localtime(&t));
	fprintf(stdout, "%-12s: %s %s\n", channel, timestr, bufout);
}

static void
sout(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	fprintf(srv, "%s\r\n", bufout);
	fflush(srv);
}

static void
privmsg(char *channel, char *msg) {
	if (channel[0] == '\0') {
		pout("", "No channel to send to");
		return;
	}
	pout(channel, "<%s> %s", nick, msg);
	sout("PRIVMSG %s :%s", channel, msg);
}

static void
handle_message(char *channel, char *s)
{
	char *out = NULL;

	if (s[0] == '!') {
		asprintf(&out, "ygor: !");
		goto finish;
	}

	if (regexec(&ygor_preg, s, 0, 0, 0) == 0) {
		memcpy(s, "ygor", 4);
		out = strdup(s);
		goto finish;
	}

	if (regexec(&lmytfy_preg, s, 0, 0, 0) == 0) {
		s += 2;
		memcpy(s, "ygor", 4);
		out = strdup(s);
		goto finish;
	}

	if (strcmp(s, "lmytfy: reload") == 0) {
		pout("*", "reloading...");
		char *fdenv;
		if (asprintf(&fdenv, "SOCKETFD=%d", fileno(srv)) == -1) {
			err(1, "failed to generate SOCKETFD pre-exec");
		}
		char const *env[] = { fdenv, NULL };
		execle(progname, progname, (char *)NULL, env);
		err(1, "failed to exec");
	}

finish:
	if (out != NULL) {
		privmsg(channel, out);
		free(out);
	}
}

static void
parsesrv(char *cmd) {
	char *usr, *par, *txt;

	usr = host;

	if (!cmd || !*cmd)
		return;

	if (cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if (cmd[0] == '\0')
			return;
		skip(usr, '!');
	}

	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');
	trim(par);

	if (!strcmp("PONG", cmd))
		return;

	if (!strcmp("PRIVMSG", cmd)) {
		pout(par, "<%s> %s", usr, txt);
		handle_message(par, txt);
	} else if (!strcmp("PING", cmd)) {
		sout("PONG %s", txt);
	} else {
		pout(usr, ">< %s (%s): %s", cmd, par, txt);
		if (!strcmp("NICK", cmd) && !strcmp(usr, nick))
			strlcpy(nick, txt, sizeof nick);
	}
}

int
main(int argc, char *argv[]) {
	(void)argc;
	char const *errstr;
	int n, fd;
	fd_set rd;
	struct timeval tv;

	progname = strdup(argv[0]);

	/* Compile the regexps in advance, saving half a cycle */
	if (regcomp(&ygor_preg, "^[ygor]{4}[^a-z0-9]",
	    REG_ICASE|REG_EXTENDED) != 0) {
		errx(1, "error: ygor_preg is fscked up");
	}
	if (regcomp(&lmytfy_preg, "^lmytfy[^a-z0-9]", REG_ICASE) != 0) {
		errx(1, "error: lmytfy_preg is fscked up");
	}

	/*
	 * Dial to the server. If a previous session is passing around a
	 * SOCKETFD, assume this is an opened file descriptor to the irc server
	 * socket.
	 */
	char const *fdc = getenv("SOCKETFD");
	if (fdc == NULL) {
		srv = fdopen(dial(host, port), "r+");
		if (!srv) {
			err(1, "error: fdopen failed: ");
		}

		sout("NICK %s", nick);
		sout("USER %s localhost %s :%s", nick, host, name);
		fflush(srv);
		sout("JOIN #dev");
		sout("JOIN #ops");
		fflush(srv);
		setbuf(stdout, NULL);
		setbuf(srv, NULL);
	} else {
		fd = strtonum(fdc, 1, 4096, &errstr);
		if (errstr) {
			errx(1, "bad value for SOCKETFD %s: %s", errstr, fdc);
		}
		srv = fdopen(fd, "r+");
	}

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
			sout("PING %s", host);
			continue;
		}

		if (FD_ISSET(fileno(srv), &rd)) {
			if (fgets(bufin, sizeof(bufin), srv) == NULL) {
				err(1, "error: server closed connection");
			}
			parsesrv(bufin);
			trespond = time(NULL);
		}
	}

	return (0);
}
