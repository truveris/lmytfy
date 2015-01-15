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

#include <stdio.h>
#include <time.h>

#include "fade.h"
#include "lmytfy.h"

#define NSECFADE 400000000;
#define FORMAT "ygor: volume %u%%"

void
fadein(char *channel)
{
	int i;
	struct timespec timeout;
	char buf[32];

	timeout.tv_sec = 0;
	timeout.tv_nsec = NSECFADE;

	for (i = 50; i <= 80; i += 5) {
		snprintf(buf, 32, FORMAT, i);
		privmsg(channel, buf);
		nanosleep(&timeout, NULL);
	}
}

void
fadeout(char *channel)
{
	int i;
	struct timespec timeout;
	char buf[32];

	timeout.tv_sec = 0;
	timeout.tv_nsec = NSECFADE;

	for (i = 80; i >= 50; i -= 5) {
		snprintf(buf, 32, FORMAT, i);
		privmsg(channel, buf);
		nanosleep(&timeout, NULL);
	}

	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;

	privmsg(channel, "ygor: stop");
	nanosleep(&timeout, NULL);

	privmsg(channel, "ygor: volume 80%");
	nanosleep(&timeout, NULL);
}
