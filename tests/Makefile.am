
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
	naninf-rnd.vgtest naninf-rnd.stderr.exp naninf-rnd.stdout.exp  \
	accuClreq1.vgtest accuClreq1.stderr.exp accuClreq1.stdout.exp  \
	accuClreq2.vgtest accuClreq2.stderr.exp accuClreq2.stdout.exp \
	denorm1.vgtest    denorm1.stderr.exp    denorm1.stdout.exp \
	denorm2.vgtest    denorm2.stderr.exp    denorm2.stdout.exp

check_PROGRAMS = clreq accuClreq sum

check_PROGRAMS += naninf
naninf_SOURCES = naninf.cxx

check_PROGRAMS += denorm
denorm_SOURCES = denorm.cxx

AM_CFLAGS   += $(AM_FLAG_M3264_PRI) -mfma -lm -O0 -g3
AM_CXXFLAGS += $(AM_FLAG_M3264_PRI) -mfma -std=c++11 -O0 -g3 -lm
