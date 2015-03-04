/*
 * Copyright 2015, Truveris Inc. All Rights Reserved.
 *
 * MIT/X Consortium License
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
#include "parse.h"
#include "irc.h"
#include "log.h"

extern char *name;
extern char *nick;
extern char *host;
extern FILE *srv;

static char bufout[4096];

extern const char *channels[];

void
irc_printf(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	fprintf(srv, "%s\r\n", bufout);
	fflush(srv);
}

void
irc_privmsg(char *channel, char *fmt, ...) {
	va_list ap;

	if (channel[0] == '\0') {
		log_printf("", "No channel to send to");
		return;
	}

	log_printf(channel, "<%s> %s", nick, bufout);

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	fprintf(srv, "PRIVMSG %s :%s\r\n", channel, bufout);
	fflush(srv);
}

/*
 * irc_parse is called for every single line received from the IRC server.
 */
void
irc_parse(char *cmd) {
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
		log_printf(par, "<%s> %s", usr, txt);
		handle_message(usr, par, txt);
	} else if (!strcmp("PING", cmd)) {
		irc_printf("PONG %s", txt);
	} else {
		log_printf(usr, ">< %s (%s): %s", cmd, par, txt);
		if (!strcmp("NICK", cmd) && !strcmp(usr, nick))
			strlcpy(nick, txt, sizeof nick);
	}
}

/*
 * irc_startup is used early in the connection process to send the USER/AUTH to
 * the server and auto-join all the configured channels.
 */
void
irc_startup(void)
{
	const char **channel;

	irc_printf("NICK %s", nick);
	irc_printf("USER %s localhost %s :%s", nick, host, name);
	fflush(srv);

	for (channel = channels; *channel != NULL; channel++) {
		irc_printf("JOIN %s", *channel);
	}
	fflush(srv);
}
