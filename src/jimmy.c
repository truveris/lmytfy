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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jimmy.h"
#include "lmytfy.h"
#include "parse.h"
#include "strlcpy.h"
#include "xmalloc.h"

static char *last_alias = NULL;
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

/*
 * If jimmy aliases something, undo it.  This reacts to ygor's response.
 */
char *
handle_jimmy_fuckup(char *msg)
{
	char *value, *output = NULL;
	time_t now;

	now = time(NULL);

	/* No previous alias. */
	if (last_alias == NULL)
		return NULL;

	/* Too old, don't matter. */
	if (now - last_alias_time > 3)
		return NULL;

	value = get_value_after_prefix(msg, "ok (replaces \"");
	if (value != NULL) {
		xasprintf(&output, "ygor: alias %s \"%s\"", last_alias, value);
		goto done;
	}

	value = get_value_after_prefix(msg, "ok (created as \"");
	if (value != NULL) {
		xasprintf(&output, "ygor: unalias %s", value);
		goto done;
	}

done:
	if (last_alias != NULL) {
		xfree(last_alias);
		last_alias = NULL;
	}

	return output;
}

/*
 * Record the last time jimmy tried to alias something, we'll use that to undo
 * it when ygor reacts.
 */
char *
handle_jimmy_message(char *msg)
{
	char *alias;

	if (addressed_to_ygor_or_typo(msg)) {
		alias = get_alias_from_msg(msg);
		if (alias == NULL)
			return NULL;
		if (last_alias != NULL)
			xfree(last_alias);
		last_alias = alias;
		last_alias_time = time(NULL);
	}

	return NULL;
}
