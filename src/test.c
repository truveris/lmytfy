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

#include <stdlib.h>
// #include <sys/param.h>
// 
// #include <stdio.h>
// #include <assert.h>
// #include <string.h>
// #include <wchar.h>
#include <locale.h>
// #include <stdarg.h>
// #include <errno.h>
// 
// #include "main.c"
// #include "utils.h"
// #include "strlcpy.h"
// #include "wcslcpy.h"
// #include "template.h"
#include "main.c"
#include "parse.h"
#include "test.h"


static int
test_addressed_to_ygor_typo__not_typo(void)
{
	size_t i;
	i = addressed_to_ygor_typo("foo: bar");
	return (assert_size_t_equals(i, 0));
}

static int
test_addressed_to_ygor_typo__long_typo(void)
{
	size_t i;
	i = addressed_to_ygor_or_typo("rogy: foobar");
	return (assert_size_t_equals(i, 6));
}

static int
test_addressed_to_ygor_typo__short_typo(void)
{
	size_t i;
	i = addressed_to_ygor_or_typo("goo: foobar");
	return (assert_size_t_equals(i, 5));
}

static int
test_addressed_to_ygor_or_typo__not_ygor(void)
{
	size_t i;
	i = addressed_to_ygor_or_typo("foo: bar");
	return (assert_size_t_equals(i, 0));
}

static int
test_addressed_to_ygor_or_typo__ygor_long(void)
{
	size_t i;
	i = addressed_to_ygor_or_typo("ygor: foobar");
	return (assert_size_t_equals(i, 6));
}

static int
test_addressed_to_ygor_or_typo__short_typo(void)
{
	size_t i;
	i = addressed_to_ygor_or_typo("goo: foobar");
	return (assert_size_t_equals(i, 5));
}

static int
test_get_alias_from_msg__full(void)
{
	char *c;
	c = get_alias_from_msg("ygor: alias foo bar");
	return (assert_string_equals(c, "foo"));
}

static int
test_get_alias_from_msg__partial(void)
{
	char *c;
	c = get_alias_from_msg(" alias foo bar");
	return (assert_string_equals(c, "foo"));
}

static int
test_get_alias_from_msg__null(void)
{
	char *c;
	c = get_alias_from_msg(" foo bar");
	return (assert_null(c));
}

static int
test_handle_typoed_ygor_message__normal(void)
{
	char *out;
	out = handle_typoed_ygor_message("ygor:anything", 5);
	return (assert_string_equals(out, "ygor: anything"));
}

static int
test_handle_typoed_ygor_message__typo(void)
{
	char *out;
	out = handle_typoed_ygor_message("rogy: anything", 6);
	return (assert_string_equals(out, "ygor: anything"));
}

static int
test_handle_typoed_ygor_message__short(void)
{
	char *out;
	out = handle_typoed_ygor_message("rog: anything", 5);
	return (assert_string_equals(out, "ygor: anything"));
}

int
main(int argc, const char *argv[])
{
	(void)argc;
	(void)argv;
	setlocale(LC_ALL, "");

	init_regexes();

	RUN_TEST(test_addressed_to_ygor_typo__not_typo);
	RUN_TEST(test_addressed_to_ygor_typo__long_typo);
	RUN_TEST(test_addressed_to_ygor_typo__short_typo);
	RUN_TEST(test_addressed_to_ygor_or_typo__not_ygor);
	RUN_TEST(test_addressed_to_ygor_or_typo__ygor_long);
	RUN_TEST(test_addressed_to_ygor_or_typo__short_typo);
	RUN_TEST(test_get_alias_from_msg__full);
	RUN_TEST(test_get_alias_from_msg__partial);
	RUN_TEST(test_get_alias_from_msg__null);
	RUN_TEST(test_handle_typoed_ygor_message__normal);
	RUN_TEST(test_handle_typoed_ygor_message__typo);
	RUN_TEST(test_handle_typoed_ygor_message__short);

	printf("%d tests (%d PASS, %d FAIL)\n", tested, passed, failed);

	if (failed) {
		return (1);
	} else {
		return (0);
	}
}
