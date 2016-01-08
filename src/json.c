/*
 * Copyright (c) 2016 Bertrand Janin <b@janin.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "json.h"

/*
 * json_string_encode() is in no way a proper JSON encoding function, but since
 * the only thing we need is to encode a JSON string, that's gonna have to do.
 */
char *
json_string_encode(const char *src)
{
	char *dst, *dp;

	/* If every letter is escaped, we need 2 times the memory. */
	if ((dst = calloc(2, strlen(src) + 1)) == NULL)
		return (NULL);

	for (dp = dst; *src != '\0'; src++) {
		if (*src == '"' || *src == '\\') {
			*dp++ = '\\';
		}
		*dp++ = *src;
	}

	return (dst);
}
