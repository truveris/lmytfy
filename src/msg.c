/*
 * MIT/X Consortium License
 *
 * Copyright 2015, Truveris Inc. All Rights Reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <time.h>

#include "fade.h"
#include "lmytfy.h"
#include "parse.h"
#include "msg.h"
#include "xmalloc.h"

char *
handle_typoed_ygor_message(char *msg, size_t offset)
{
	char *out;

	if (strncmp(msg, "orgy", 4) == 0) {
		return strdup("ಠ_ಠ");
	}

	msg += offset;
	xasprintf(&out, "ygor: %s", msg);

	return out;
}

/*
 * A lmytfy command may be called.
 */
static char *
handle_commands(char *channel, char *msg)
{
	char *s, *out = NULL;

	s = strstr(msg, "fade");
	if (s != NULL)
		msg = s;

	if (streq(msg, "fadein")) {
		fadein(channel);
	} else if (streq(msg, "fadeout")) {
		fadeout(channel);
	} else if (streq(msg, "++") || streq(msg, "--")) {
		// NOP
	} else {
		out = strdup("\001ACTION error: invalid command\001");
	}

	return out;
}

void
handle_message(char *user, char *channel, char *msg)
{
	char *out = NULL;
	char alias[MAXIRCLEN], value[MAXIRCLEN];
	size_t offset;

	if (msg[0] == '!') {
		out = strdup("ygor: !");
		goto done;
	}

	if ((offset = addressed_to_ygor_or_typo(msg)) > 0) {
		if (is_short_imgur(msg + offset)) {
			sleep(1);
			xasprintf(&out, "ygor: %s.gif", msg + offset);
			goto done;
		}

		if (is_gifv_imgur(msg + offset)) {
			sleep(1);
			*(msg + offset + strlen(msg + offset) - 1) = '\0';
			xasprintf(&out, "ygor: %s", msg + offset);
			goto done;
		}

		if ((offset = addressed_to_ygor_typo(msg)) > 0) {
			out = handle_typoed_ygor_message(msg, offset);
			msg = out;
		}

		if (parse_alias(msg, alias, value)) {
			out = handle_alias(user, alias, value);
			goto done;
		}
	}

	if (streq(user, "ygor")) {
		out = handle_ygor_msg(msg);
		goto done;
	}

	if (addressed_to_lmytfy(msg)) {
		out = handle_commands(channel, msg);
		goto done;
	}

done:
	if (out != NULL) {
		irc_privmsg(channel, "%s", out);
		xfree(out);
	}
}
