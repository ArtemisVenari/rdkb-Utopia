bin_PROGRAMS = naptr_parser
AM_CFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS = -I$(top_srcdir)/source -I$(top_srcdir)/source/include $(CPPFLAGS) $(DBUS_CFLAGS)
AM_LDFLAGS =  -lcares
naptr_parser_SOURCES =  naptr_parser.c

naptr_parser_LDADD = $(top_builddir)/source/syscfg/lib/libsyscfg.la \
                     $(top_builddir)/source/sysevent/lib/libsysevent.la
