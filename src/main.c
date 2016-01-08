/*
 * MIT/X Consortium License
 *
 * Copyright 2014-2016, Truveris Inc. All Rights Reserved.
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
#include <stdio.h>

#include "irc.h"
#include "mattermost.h"
#include "log.h"
#include "parse.h"

extern char *__progname;

#ifndef REGRESS
int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	init_regexes();

	if (streq(__progname, "lmytfy-irc")) {
		lmytfy_irc();
	} else if (streq(__progname, "lmytfy-mattermost")) {
		lmytfy_mattermost();
	} else {
		errx(1, "unknown program name");
	}

	return (0);
}
#endif // ifndef REGRESS
