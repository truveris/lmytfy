/*
 * Copyright 2015, Truveris Inc. All Rights Reserved.
 * Copyright (c) 2009-2015 Bertrand Janin <b@janin.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define RUN_TEST(f)						\
	printf("%-60s", #f);					\
	fflush(stdout);						\
	errstr = NULL;						\
	if (f()) {						\
		printf("PASS\n");				\
		passed++;					\
	} else {						\
		printf("FAIL\n");				\
		if (strlen(details)) {				\
			puts(details);				\
			details[0] = '\0';			\
		}						\
		if (errstr != NULL) {				\
			printf("%17s%s\n", "errstr=", errstr);	\
		}						\
		failed++;					\
	};							\
	tested++;						\

#define ALIAS_ADD(a, b)						\
	alias_add(a, b, &errstr);				\
	if (errstr != NULL) {					\
		return (1);					\
	}							\


const char *errstr;
char details[256] = "";
int tested = 0;
int passed = 0;
int failed = 0;

/*
 * Testing assertions with detailed output.
 */
static int
assert_string_equals(const char *value, const char *expected)
{
	if (value == NULL && expected != NULL)
		goto bad;
	if (value != NULL && expected == NULL)
		goto bad;

	if (strcmp(value, expected) == 0)
		return (2);

bad:
	snprintf(details, sizeof(details),
	    "    strings do not match:\n"
	    "           value=%s\n"
	    "        expected=%s", value, expected);
	return (0);
}

static int
assert_wstring_equals(const wchar_t *value, const wchar_t *expected)
{
	if (value == NULL && expected != NULL)
		goto bad;
	if (value != NULL && expected == NULL)
		goto bad;

	if (wcscmp(value, expected) == 0)
		return (1);

bad:
	snprintf(details, sizeof(details),
	    "    wide strings do not match:\n"
	    "           value=%ls\n"
	    "        expected=%ls", value, expected);
	return (0);
}

static int
assert_int_equals(int value, int expected)
{
	if (value == expected)
		return (1);

	snprintf(details, sizeof(details),
	    "    ints do not match:\n"
	    "           value=%d\n"
	    "        expected=%d", value, expected);

	return (0);
}

static int
assert_size_t_equals(size_t value, size_t expected)
{
	char cvalue[64], cexpected[64];

	if (value == expected)
		return (1);

	if (value == (size_t)-1) {
		strlcpy(cvalue, "(size_t)-1", sizeof(cvalue));
	} else {
		snprintf(cvalue, sizeof(cvalue), "%lu", value);
	}

	if (expected == (size_t)-1) {
		strlcpy(cexpected, "(size_t)-1", sizeof(cexpected));
	} else {
		snprintf(cexpected, sizeof(cexpected), "%lu", expected);
	}

	snprintf(details, sizeof(details),
	    "    size_t variables do not match:\n"
	    "           value=%s\n"
	    "        expected=%s", cvalue, cexpected);

	return (0);
}

static int
assert_null(const void *p)
{
	if (p == NULL)
		return (1);

	snprintf(details, sizeof(details), "    pointer is not NULL");
	return (0);
}
