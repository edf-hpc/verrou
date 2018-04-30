
include $(top_srcdir)/Makefile.tool-tests.am

SUBDIRS = .
DIST_SUBDIRS = .

dist_noinst_SCRIPTS = filter_stderr

EXTRA_DIST = \
	clreq.vgtest      clreq.stderr.exp      clreq.stdout.exp \
	sum1.vgtest       sum1.stderr.exp       sum1.stdout.exp  \
	sum2.vgtest       sum2.stderr.exp       sum2.stdout.exp  \
	sum3.vgtest       sum3.stderr.exp       sum3.stdout.exp  \
	seed.vgtest       seed.stderr.exp       seed.stdout.exp  \
	naninf-rnd.vgtest naninf-rnd.stderr.exp naninf-rnd.stdout.exp

check_PROGRAMS = clreq sum

check_PROGRAMS += naninf
naninf_SOURCES = naninf.cxx

AM_CFLAGS   += $(AM_FLAG_M3264_PRI)
AM_CXXFLAGS += $(AM_FLAG_M3264_PRI) -std=c++11
