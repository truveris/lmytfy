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

static char last_alias_value[MAXIRCLEN];
static char last_alias_user[MAXIRCLEN];
static char last_alias[MAXIRCLEN];
static time_t last_alias_time;

/*
 * Attempt to extract the value from an ygor:alias return after the provided
 * prefix.
 */
static char *
get_value_after_prefix(char *s, char *prefix)
{
	char *value = NULL;
	size_t len;

	if (startswith(s, prefix)) {
		s += strlen(prefix);
		len = strlen(s);

		/* Skip trailing quote and paren, ignore empty aliases */
		if (len <= 2)
			return NULL;
		len -= 2;

		value = malloc(len + 1);
		strlcpy(value, s, len + 1);
	}

	return value;
}

static char *
fix_short_imgur_alias(char *alias, char *value)
{
	char *output;
	xasprintf(&output, "ygor: alias %s %s.gif", alias, value);
	return output;
}

static char *
fix_gifv_imgur_alias(char *alias, char *value)
{
	char *output;
	*(value + strlen(value) - 1) = '\0';
	xasprintf(&output, "ygor: alias %s %s", alias, value);
	return output;
}

/*
 * Handle messages from ygor.
 */
char *
handle_ygor_msg(char *msg)
{
	char *value, *output = NULL;
	time_t now;

	now = time(NULL);

	/* Too old, don't matter. */
	if (now - last_alias_time > 3)
		return NULL;

	if (streq(msg, "ok")) {
		if (is_short_imgur(last_alias_value)) {
			output = fix_short_imgur_alias(last_alias,
					last_alias_value);
			goto done;
		}
		if (is_gifv_imgur(last_alias_value)) {
			output = fix_gifv_imgur_alias(last_alias,
					last_alias_value);
			goto done;
		}
	}

	value = get_value_after_prefix(msg, "ok (replaces \"");
	if (value != NULL) {
		if (streq(last_alias_user, "jimmy")) {
			xasprintf(&output, "ygor: alias %s \"%s\"", last_alias,
			    value);
			goto done;
		}
		if (is_short_imgur(last_alias_value)) {
			output = fix_short_imgur_alias(last_alias,
					last_alias_value);
			goto done;
		}
		if (is_gifv_imgur(last_alias_value)) {
			output = fix_gifv_imgur_alias(last_alias,
					last_alias_value);
			goto done;
		}
	}

	value = get_value_after_prefix(msg, "ok (created as \"");
	if (value != NULL) {
		if (streq(last_alias_user, "jimmy")) {
			xasprintf(&output, "ygor: unalias %s", value);
			goto done;
		}
		if (is_short_imgur(last_alias_value)) {
			output = fix_short_imgur_alias(value,
					last_alias_value);
			goto done;
		}
		if (is_gifv_imgur(last_alias_value)) {
			output = fix_gifv_imgur_alias(value, last_alias_value);
			goto done;
		}
	}

done:
	last_alias_time = 0;

	return output;
}

/*
 * Handle mistakes of normal ygor users.
 */
char *
handle_alias(char *user, char *alias, char *value)
{
	char *out = NULL;

	/* Keep track of all aliases in case we need to correct. */
	strlcpy(last_alias, alias, MAXIRCLEN);
	strlcpy(last_alias_user, user, MAXIRCLEN);
	strlcpy(last_alias_value, value, MAXIRCLEN);
	last_alias_time = time(NULL);

	return out;
}
