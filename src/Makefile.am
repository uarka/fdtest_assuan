SUBDIRS = .
AM_CPPFLAGS = $(LIBASSUAN_CFLAGS)
LDADD = $(LIBASSUAN_LIBS)

bin_PROGRAMS = fdtest_client fdtest_server fdtest_client_inquire fdtest_server_inquire \
	       fdtest_client_out fdtest_server_out
fdtest_client_SOURCES = fdtest_client.c
fdtest_server_SOURCES = fdtest_server.c
fdtest_client_inquire_SOURCES = fdtest_client_inquire.c
fdtest_server_inquire_SOURCES = fdtest_server_inquire.c
fdtest_client_out_SOURCES = fdtest_client_out.c
fdtest_server_out_SOURCES = fdtest_server_out.c

