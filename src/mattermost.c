/*
 * MIT/X Consortium License
 *
 * Copyright 2016 (c) Bertrand Janin <b@janin.com>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <err.h>

#include "log.h"
#include "json.h"
#include "mattermost.h"
#include "msg.h"
#include "url.h"
#include "xmalloc.h"

#define WEBHOOK_PORT 3333
#define BUFSIZE 4096
#define RESPONSE_TEMPLATE \
	"HTTP/1.1 200 OK\r\n" \
	"Content-Type: application/json\r\n" \
	"Content-Length: %d\r\n" \
	"\r\n" \
	"%s"
#define JSON_TEMPLATE \
	"{" \
	"\"username\": \"lmytfy\"," \
	"\"icon_url\": \"https://s3.amazonaws.com/truveris-mattermost-icons/troll.png\"," \
	"\"text\": \"I think you meant to do `%s`\"" \
	"}"

static void
http_response(int fd, const char *json)
{
	int n;
	char *s;

	if (json == NULL) {
		json = "{}";
	}

	xasprintf(&s, RESPONSE_TEMPLATE, strlen(json), json);

	n = write(fd, s, strlen(s));
	if (n < 0) {
		log_printf("-", "mattermost: response write(header) failed");
	}

	xfree(s);
}

static char *
get_json_from_response(const char *response)
{
	char *jsonstr, *json;

	jsonstr = json_string_encode(response);
	xasprintf(&json, JSON_TEMPLATE, jsonstr);
	xfree(jsonstr);

	return (json);
}

void
lmytfy_mattermost(void)
{
	int parentfd;
	int childfd;
	socklen_t clientlen;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	struct hostent *hostp;
	static char buf[BUFSIZE];
	char *hostaddrp;
	char *body, *text, *eot, *response, *json;
	int optval;
	int n;

	parentfd = socket(AF_INET, SOCK_STREAM, 0);
	if (parentfd < 0) {
		err(1, "socket() failed");
	}

	/*
	 * Debugging trick that lets us rerun the server immediately after we
	 * kill it; otherwise we have to wait about 20 secs.
	 */
	optval = 1;
	setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
			 (const void *)&optval , sizeof(int));

	/* Build the server address. */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)WEBHOOK_PORT);

	/* Associate the parent socket with a port. */
	if (bind(parentfd, (struct sockaddr *) &serveraddr,
		 sizeof(serveraddr)) < 0) {
		err(1, "bind() failed");
	}

	/* Make the socket ready to accept a few connections. */
	if (listen(parentfd, 16) < 0) {
		err(1, "listen() failed");
	}

	clientlen = sizeof(clientaddr);
	for (;;) {
		/* Wait for a connection request. */
		childfd = accept(parentfd, (struct sockaddr *) &clientaddr,
				&clientlen);
		if (childfd < 0) {
			err(1, "accept() failed");
		}

		/* Determine who sent the message. */
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
				sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL) {
			err(1, "gethostbyaddr() failed");
		}
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL) {
			err(1, "inet_ntoa() failed");
		}
		log_printf("-", "mattermost: server established connection "
				"with %s (%s)", hostp->h_name, hostaddrp);

		/* Read the input data from the client. */
		n = read(childfd, buf, BUFSIZE);
		if (n < 0) {
			err(1, "read() from socket failed");
		} else if (n >= BUFSIZE) {
			log_printf("-", "mattermost: bad request (too long)");
			http_response(childfd, NULL);
			goto end_of_request;
		}
		log_printf("-", "mattermost: server received %d bytes", n);

		/* Ensure that we have a terminaing NUL-byte. */
		buf[n] = '\0';

		/* Jump to the body of the request. */
		body = strstr(buf, "\r\n\r\n");
		if (body == NULL) {
			log_printf("-", "mattermost: bad request (no body)");
			http_response(childfd, NULL);
			goto end_of_request;
		}
		body += 4;

		/* Isolate the text parameter. */
		text = strstr(body, "text=");
		if (text == NULL) {
			log_printf("-", "mattermost: bad request (no text)");
			http_response(childfd, NULL);
			goto end_of_request;
		}
		text += 5;

		eot = strchr(text, '&');
		if (eot != NULL) {
			*eot = '\0';
		}

		url_decode(text);
		log_printf("-", "mattermost: text: %s", text);

		/* Get the response from the lmytfy core. */
		response = handle_message("", "", text);
		if (response == NULL) {
			http_response(childfd, NULL);
			goto end_of_request;
		}

		json = get_json_from_response(response);
		xfree(response);

		/* Respond to the client. */
		log_printf("-", "mattermost: response: %s", json);
		http_response(childfd, json);
		xfree(json);

end_of_request:
		close(childfd);
	}
}
