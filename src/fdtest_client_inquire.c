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
#include <string.h>

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

#define SOURCE_FILE	"/tmp/sourcefile.txt"
#define SOCKET_FILE	"/tmp/S.assuan"

assuan_context_t ctx;

static gpg_error_t trans_data(void *p, const void *data, size_t len)
{
	fprintf(stderr, data);
	return 0;
}

static gpg_error_t trans_inquire(void *p, const char *line)
{
	gpg_error_t err;

	err = assuan_send_data(ctx, "D TEST1", strlen("D TEST1"));
	print_err(err);
	err = assuan_send_data(ctx, "D TEST2", strlen("D TEST2"));
	print_err(err);
	err = assuan_send_data(ctx, "EST3", strlen("EST3"));
	print_err(err);
	//err = assuan_send_data(ctx, NULL, 0);
	//print_err(err);
	//err = assuan_write_line(ctx, "END");
	//print_err(err);
	return 0;
}

static gpg_error_t trans_status(void *p, const char *line)
{
	return 0;
}

/* First create the context and create the file descriptor for the file
 *
 * Send command to the server in the following order
 *   Send the file descriptor
 *   Send the ECHO command
 */
int main(int argc, char *argv[])
{
	size_t len;
	gpg_error_t rc;
	assuan_fd_t fd;


	/* Create the context */
	rc = assuan_new(&ctx);
	if (rc)
	{
		fprintf(stderr, "can't create context: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Establish a connection to the assuan server */
	rc = assuan_socket_connect(ctx, SOCKET_FILE, ASSUAN_INVALID_PID,
			ASSUAN_SOCKET_CONNECT_FDPASSING);
	if (rc)
	{
		fprintf(stderr, "can't connect to socket: %s\n",
				gpg_strerror(rc));
		exit(1);
	}

	/* Send the ECHO command */
	rc = assuan_transact(ctx, "ECHO", trans_data, NULL,
			trans_inquire, NULL,
			trans_status, NULL);
	if (rc)
	{
		fprintf(stderr, "can't transact ECHO: %s\n",
				gpg_strerror(rc));
		exit(1);
	}
	/* Send the ACT command */
	rc = assuan_transact(ctx, "ACT", trans_data, NULL,
			trans_inquire, NULL,
			trans_status, NULL);
	if (rc)
	{
		fprintf(stderr, "can't transact ACT: %s\n",
				gpg_strerror(rc));
		exit(1);
	}


	/* Cleanup */
	assuan_release(ctx);
}
