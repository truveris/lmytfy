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

#include "lmytfy.h"
#include "parse.h"
#include "jimmy.h"
#include "msg.h"
#include "xmalloc.h"

static char *
handle_typoed_ygor_message(char *msg)
{
	if (strncmp(msg, "orgy", 4) == 0) {
		return strdup("ಠ_ಠ");
	}

	memcpy(msg, "ygor", 4);
	return strdup(msg);
}

/*
 * Pass all the messages sent to lmytfy to ygor.
 */
static char *
handle_ygor_forward(char *msg)
{
	msg += 2;
	memcpy(msg, "ygor", 4);
	return strdup(msg);
}

void
handle_message(char *user, char *channel, char *msg)
{
	char *out = NULL;

	if (msg[0] == '!') {
		out = strdup("ygor: !");
		goto done;
	}

	if (strcmp(user, "jimmy") == 0) {
		out = handle_jimmy_message(msg);
		goto done;
	}

	if (addressed_to_ygor_typo(msg)) {
		out = handle_typoed_ygor_message(msg);
		goto done;
	}

	if (strcmp(user, "ygor") == 0) {
		out = handle_jimmy_fuckup(msg);
		goto done;
	}

	if (addressed_to_lmytfy(msg)) {
		out = handle_ygor_forward(msg);
		goto done;
	}

done:
	if (out != NULL) {
		privmsg(channel, out);
		xfree(out);
	}
}
