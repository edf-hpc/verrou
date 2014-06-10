
include $(top_srcdir)/Makefile.tool-tests.am

SUBDIRS = .
DIST_SUBDIRS = .

dist_noinst_SCRIPTS = filter_stderr

EXTRA_DIST = \
	clreq.vgtest clreq.stderr.exp clreq.stdout.exp \
	sum1.vgtest  sum1.stderr.exp  sum1.stdout.exp  \
	sum2.vgtest  sum2.stderr.exp  sum2.stdout.exp  \
	sum3.vgtest  sum3.stderr.exp  sum3.stdout.exp

check_PROGRAMS = clreq sum

AM_CFLAGS   += $(AM_FLAG_M3264_PRI)
AM_CXXFLAGS += $(AM_FLAG_M3264_PRI)
