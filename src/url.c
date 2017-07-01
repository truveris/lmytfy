/*
 * Copyright (c) 2014 Reyk Floeter <reyk@openbsd.org>
 * Copyright (c) 2014 Reyk Floeter <reyk@openbsd.org>
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
#include <string.h>
#include <ctype.h>

#include "url.h"

const char *
url_decode(char *url)
{
	char		*p, *q;
	char		 hex[3];
	unsigned long	 x;

	hex[2] = '\0';
	p = q = url;

	while (*p != '\0') {
		switch (*p) {
		case '+':
			*q = ' ';
			break;
		case '%':
			/* Encoding character is followed by two hex chars */
			if (!(isxdigit((unsigned char)p[1]) &&
			    isxdigit((unsigned char)p[2])))
				return (NULL);

			hex[0] = p[1];
			hex[1] = p[2];

			/*
			 * We don't have to validate "hex" because it is
			 * guaranteed to include two hex chars followed by nul.
			 */
			x = strtoul(hex, NULL, 16);
			*q = (char)x;
			p += 2;
			break;
		default:
			*q = *p;
			break;
		}
		p++;
		q++;
	}
	*q = '\0';

	return (url);
}
