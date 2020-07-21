/*
 * Copyright (c) 2020 Felicity Janet Meadows
 * Copyright (c) 2001-2013 Free Software Foundation, Inc
 * Copyright (c) 2001-2015 g10 Code GmbH
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Large sections copied from book 'Developing with Assuan'
 *   by Werner Koch and Marcus Brinkmann
 *   https://gnupg.org/documentation/manuals/assuan.pdf
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <assuan.h>
#include <gpgrt.h>
#define SOCKET_FILE "/tmp/S.assuan"

/* Our command handler structure with the connection between the
 * command and it's associated function.
 */
static unsigned int echo_cmd(assuan_context_t ctx, char *line);
static unsigned int act_cmd(assuan_context_t ctx, char *line);

static struct cmd_handler {
	const char *name;
	unsigned int (*handler)(assuan_context_t ctx, char *line);
} command_table[] = {
	{ "ECHO", echo_cmd },
	{ "ACT", act_cmd },
	{ "INPUT", NULL },
	{ "OUTPUT", NULL },
	{ NULL, NULL }
};

/* Our command handler for ECHO */
unsigned int echo_cmd(assuan_context_t ctx, char *line)
{
	gpg_error_t rc;
	unsigned char *buf;
	size_t len;

	rc = assuan_inquire(ctx, "INQUIRE TEST", &buf, &len, 1000);
	if (rc)
	{
		fprintf(stderr, "inquire failed: %s\n",
				gpg_strerror(rc));
		return 1;
	}
	fprintf(stderr, buf);


 
	return 0;
}

unsigned int act_cmd(assuan_context_t ctx, char *line)
{
	gpg_error_t rc;
	unsigned char *buf;
	size_t len;

	rc = assuan_send_data(ctx, "act", 3);
	if (rc)
	{
		fprintf(stderr, "send data failed: %s\n",
				gpg_strerror(rc));
		return 1;
	}
	fprintf(stderr, buf);


 
	return 0;
}


/* Create a new socket S.assuan and create a new assuan server
 * context to serve on the socket. The command handlers are
 * registered and then the various client command are accepted and
 * processed.
 */
int main(int argc, char *argv[])
{
	char *sock_path = SOCKET_FILE;
	struct sockaddr_un sock_addr;
	struct sockaddr *sock_ptr = (struct sockaddr *)&sock_addr;
	int r_redirect;
	size_t len;
	assuan_fd_t sock_fd, l_fd;
	assuan_context_t ctx;
	int i;

	gpg_error_t rc;

	/* Delete and create a new socket */
	unlink(sock_path);
	assuan_sock_init();
	assuan_sock_set_sockaddr_un(sock_path, sock_ptr, &r_redirect);
	sock_fd = assuan_sock_new(AF_UNIX, SOCK_STREAM, 0);
	len = sizeof(sock_addr.sun_family) + strlen(sock_addr.sun_path);
	assuan_sock_bind(sock_fd, sock_ptr, len);
	listen(sock_fd, 3);

	len = sizeof(sock_addr.sun_family) + strlen(sock_addr.sun_path);
	l_fd = accept(sock_fd, sock_ptr, (socklen_t * restrict)&len);

	/* New context */
	rc = assuan_new(&ctx);
	if (rc)
	{
		fprintf(stderr, "server context creation failed: %s\n",
				gpg_strerror(rc));
		return 1;
	}

	/* Connect to the socket as a socket server */
	rc = assuan_init_socket_server(ctx, l_fd,
			ASSUAN_SOCKET_SERVER_ACCEPTED |
			ASSUAN_SOCKET_SERVER_FDPASSING);
	if (rc)
	{
		fprintf(stderr, "server init failed: %s\n",
				gpg_strerror(rc));
		return 1;
	}

	/* Register the command functions */
	for (i = 0; command_table[i].name; i++)
	{
		rc = assuan_register_command (ctx,
				command_table[i].name,
				command_table[i].handler, NULL);
		if (rc)
		{
			fprintf (stderr, "register failed: %s\n",
					gpg_strerror (rc));
			assuan_release (ctx);
			return 1;
		}
	}

	/* Our main loop processing commands */
	for (;;)
	{
		rc = assuan_accept (ctx);
 		if (rc == -1)
			break;
		else if (rc)
		{
			fprintf (stderr, "accept problem: %s\n",
					gpg_strerror (rc));
			break;
		}
		rc = assuan_process (ctx);
		if (rc)
		{
			fprintf (stderr, "processing failed: %s\n",
					gpg_strerror (rc));
			continue;
		}
	}

	/* Cleanup */
	assuan_release(ctx);
	assuan_sock_close(sock_fd);
	unlink(sock_path);
}
