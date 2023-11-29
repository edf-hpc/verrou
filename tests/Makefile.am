
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
	denorm2.vgtest    denorm2.stderr.exp    denorm2.stdout.exp \
	test_det_average_comdet.stderr.exp test_det_average_comdet.stdout.exp  test_det_average_comdet.vgtest \
	test_det_average_det.stderr.exp test_det_average_det.stdout.exp test_det_average_det.vgtest \
	test_det_random_comdet.stderr.exp test_det_random_comdet.stdout.exp  test_det_random_comdet.vgtest \
	test_det_random_det.stderr.exp  test_det_random_det.stdout.exp  test_det_random_det.vgtest

check_PROGRAMS = clreq accuClreq sum libmtest

check_PROGRAMS += naninf
naninf_SOURCES = naninf.cxx

check_PROGRAMS += denorm
denorm_SOURCES = denorm.cxx

check_PROGRAMS += test_det test_libm
test_det_SOURCES = test_det.cxx
test_libm_SOURCES = test_libm.cxx


VERROU_FLAG = -O0 -g3 -fno-builtin -mno-avx512f
if HAVE_FMA_INTRIN
VERROU_FLAG += -march=native -mfma -DUSE_VERROU_FMA
endif
AM_CFLAGS   += $(AM_FLAG_M3264_PRI)  $(VERROU_FLAG)
AM_CXXFLAGS += $(AM_FLAG_M3264_PRI)  -std=c++11 $(VERROU_FLAG)
LDADD  =  -lm
