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

#include <ctype.h>
#include <err.h>
#include <string.h>
#include <regex.h>

#include "lmytfy.h"
#include "parse.h"
#include "strlcpy.h"
#include "xmalloc.h"

regex_t ygor_preg, lmytfy_preg, alias_preg;

int
startswith(char *s, char *prefix)
{
	if (strncmp(s, prefix, strlen(prefix)) == 0)
		return (1);
	return (0);
}

/*
 * Small wrapper around strcmp with a lighter footprint, it returns 1 if the
 * strings are identical.
 */
int
streq(char *a, char *b)
{
	if (strcmp(a, b) == 0)
		return (1);
	return (0);
}

/*
 * Checks if the message appears to be addressed to "lmytfy".
 */
int
addressed_to_lmytfy(char *msg)
{
	if (regexec(&lmytfy_preg, msg, 0, 0, 0) == 0)
		return (1);

	return (0);
}

/*
 * Check if the message starts with ygor or a typo-ed variant. Returns the
 * number of bytes of the name or typo. For example "ygor:" returns 5.
 */
size_t
addressed_to_ygor_or_typo(const char *msg)
{
	regmatch_t m[1];

	if (regexec(&ygor_preg, msg, 1, m, 0) == 0) {
		return (m[0].rm_eo);
	}

	return (0);
}

/*
 * Check if the message is addressed to a typo-ed variant of ygor, but not ygor
 * directly. Returns the number of bytes of the typo-ed name or 0.
 */
size_t
addressed_to_ygor_typo(const char *msg)
{
	regmatch_t m[1];

	if (strncmp(msg, "ygor", 4) == 0)
		return (0);

	if (regexec(&ygor_preg, msg, 1, m, 0) != 0)
		return (0);

	return (m[0].rm_eo);
}

/*
 * Return the alias from an "ygor: " message.  The caller is responsible for
 * free'ing the memory.
 */
char *
get_alias_from_msg(char *msg)
{
	regmatch_t m[2];
	size_t len;
	char *alias;

	if (regexec(&alias_preg, msg, 2, m, 0) == 0) {
		len = m[1].rm_eo - m[1].rm_so;
		alias = xmalloc(len + 1);
		strlcpy(alias, msg + m[1].rm_so, len + 1);
		return (alias);
	}

	return (NULL);
}

char *
skip(char *s, char c) {
	while (*s != c && *s != '\0')
		s++;
	if (*s != '\0')
		*s++ = '\0';
	return (s);
}

void
trim(char *s) {
	char *e;

	e = s + strlen(s) - 1;
	while (isspace(*e) && e > s)
		e--;
	*(e + 1) = '\0';
}

/*
 * Compile the regexps in advance, saving half a precious CPU cycle.
 */
#define REG_COMP(a, b) \
	if (regcomp(&a, b, (REG_ICASE|REG_EXTENDED)) != 0) {		\
		errx(1, "error: %s is not a valid regex", #a);		\
	}
void
init_regexes(void)
{
	REG_COMP(ygor_preg,	"^[ygor]{3,4}[^a-zA-Z0-9]+");
	REG_COMP(lmytfy_preg,	"^lmytfy[^a-z0-9]");
	REG_COMP(alias_preg,	"alias ([^ ]+)");
}
