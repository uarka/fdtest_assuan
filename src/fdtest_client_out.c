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

#include <assuan.h>
#include <gpgrt.h>
#include <unistd.h>
#include <fcntl.h>

#define print_err(err)					\
	if (err)					\
	{						\
		fprintf(stderr, "%s:%d %s: %s\n",	\
				__FILE__, __LINE__,	\
				gpg_strerror(err));	\
		exit(1);				\
	}

#define DESTINATION_FILE "/tmp/destfile.txt"
#define SOCKET_FILE	"/tmp/S.assuan"

/* First create the context and create the file descriptor for the file
 *
 * Send command to the server in the following order
 *   Send the file descriptor
 *   Send the OUTPUT FD command to put the file descriptor in the output
 *           to the server 
 *   Send the ECHO command
 */
int main(int argc, char *argv[])
{
	size_t len;
	gpg_error_t rc;
	assuan_context_t ctx;
	assuan_fd_t fd;


	/* Create the context */
	rc = assuan_new(&ctx);
	if (rc)
	{
		fprintf(stderr, "can't create context: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Open the file descriptor */
	fd = open(DESTINATION_FILE, O_RDWR | O_CREAT, 0666);

	/* Establish a connection to the assuan server */
	rc = assuan_socket_connect(ctx, SOCKET_FILE, ASSUAN_INVALID_PID,
			ASSUAN_SOCKET_CONNECT_FDPASSING);
	if (rc)
	{
		fprintf(stderr, "can't connect to socket: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Send the file descriptor */
	rc = assuan_sendfd(ctx, fd);
	if (rc)
	{
		fprintf(stderr, "can't send fd: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Send the OUTPUT FD command */
	rc = assuan_transact(ctx, "OUTPUT FD", NULL, NULL,
			NULL, NULL,
			NULL, NULL);
	if (rc)
	{
		fprintf(stderr, "can't transact INPUT FD: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Send the ECHO command */
	rc = assuan_transact(ctx, "ECHO", NULL, NULL,
			NULL, NULL,
			NULL, NULL);
	if (rc)
	{
		fprintf(stderr, "can't transact ECHO: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Cleanup */
	close(fd);
	assuan_release(ctx);
}
