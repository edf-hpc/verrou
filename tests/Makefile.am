
include $(top_srcdir)/Makefile.tool-tests.am

SUBDIRS = .
DIST_SUBDIRS = .

dist_noinst_SCRIPTS = filter_stderr filter_stdout

EXTRA_DIST = \
	clreq.vgtest      clreq.stderr.exp      clreq.stdout.exp \
	clreq_both.vgtest clreq_both.stderr.exp clreq_both.stdout.exp \
	clreq_soft.vgtest clreq_soft.stderr.exp clreq_soft.stdout.exp \
	sum1.vgtest       sum1.stderr.exp       sum1.stdout.exp  \
	sum2.vgtest       sum2.stderr.exp       sum2.stdout.exp  \
	sum3.vgtest       sum3.stderr.exp       sum3.stdout.exp  \
	seed.vgtest       seed.stderr.exp       seed.stdout.exp  \
	naninf-rnd.vgtest naninf-rnd.stderr.exp naninf-rnd.stdout.exp naninf-rnd.stdout.exp.aarch64 \
	accuClreq1.vgtest accuClreq1.stderr.exp accuClreq1.stdout.exp  \
	accuClreq2.vgtest accuClreq2.stderr.exp accuClreq2.stdout.exp \
	accuClreq3.vgtest accuClreq3.stderr.exp accuClreq3.stdout.exp  \
	accuClreq4.vgtest accuClreq4.stderr.exp accuClreq4.stdout.exp \
	accuClreq5.vgtest accuClreq5.stderr.exp accuClreq5.stdout.exp \
	denorm1.vgtest    denorm1.stderr.exp    denorm1.stdout.exp denorm1.stderr.exp.aarch64 \
	denorm2.vgtest    denorm2.stderr.exp    denorm2.stdout.exp \
	test_det_average_comdet.stderr.exp test_det_average_comdet.stdout.exp  test_det_average_comdet.vgtest \
	test_det_average_scomdet.stderr.exp test_det_average_scomdet.stdout.exp  test_det_average_scomdet.vgtest \
	test_det_average_det.stderr.exp test_det_average_det.stdout.exp test_det_average_det.vgtest \
	test_det_random_comdet.stderr.exp test_det_random_comdet.stdout.exp  test_det_random_comdet.vgtest \
	test_det_random_scomdet.stderr.exp test_det_random_scomdet.stdout.exp  test_det_random_scomdet.vgtest \
	test_det_random_det.stderr.exp test_det_random_det.stdout.exp  test_det_random_det.vgtest \
	test_det_sr_monotonic.stderr.exp test_det_sr_monotonic.stdout.exp  test_det_sr_monotonic.vgtest \
	test_det_sr_smonotonic.stderr.exp test_det_sr_smonotonic.stdout.exp  test_det_sr_smonotonic.vgtest \
	libmtest-withlibm.stderr.exp libmtest-withlibm.stdout.exp libmtest-withlibm.vgtest libmtest-withlibm.stderr.exp.aarch64 \
	libmtest-withoutlibm.stderr.exp libmtest-withoutlibm.stdout.exp libmtest-withoutlibm.vgtest \
	seed_random_det.aarch64.stderr.exp seed_random_det.aarch64.stdout.exp seed_random_det.aarch64.vgtest \
	seed_random_det.x86_64.stderr.exp seed_random_det.x86_64.stdout.exp seed_random_det.x86_64.vgtest \
	test_libm_random.stderr.exp test_libm_random.stderr.exp.aarch64 test_libm_random.stdout.exp test_libm_random.vgtest \
	test_libm_random_det.stderr.exp test_libm_random_det.stderr.exp.aarch64 test_libm_random_det.stdout.exp test_libm_random_det.stdout.exp.aarch64 test_libm_random_det.vgtest \
	test_libm_random_scomdet.stderr.exp test_libm_random_scomdet.stderr.exp.aarch64 test_libm_random_scomdet.stdout.exp test_libm_random_scomdet.vgtest \
	hardSoftClreq1.stderr.exp hardSoftClreq1.stdout.exp hardSoftClreq1.vgtest \
	hardSoftClreq2.stderr.exp hardSoftClreq2.stdout.exp hardSoftClreq2.vgtest \
	hardSoftClreq3.stderr.exp hardSoftClreq3.stdout.exp hardSoftClreq3.vgtest iomatch.script \
	hardSoftClreqDouble1.stderr.exp hardSoftClreqDouble1.stdout.exp hardSoftClreqDouble1.vgtest \
	hardSoftClreqDouble2.stderr.exp hardSoftClreqDouble2.stdout.exp hardSoftClreqDouble2.vgtest \
	hardSoftClreqDouble3.stderr.exp hardSoftClreqDouble3.stdout.exp hardSoftClreqDouble3.vgtest \
	libmtest_float_conv1.stderr.exp libmtest_float_conv1.stdout.exp libmtest_float_conv1.vgtest \
	libmtest_float_conv2.stderr.exp libmtest_float_conv2.stdout.exp libmtest_float_conv2.vgtest 

check_PROGRAMS = clreq accuClreq sum libmtest hardSoftClreq hardSoftClreqDouble libmtest_float_conv

check_PROGRAMS += naninf
naninf_SOURCES = naninf.cxx

check_PROGRAMS += denorm
denorm_SOURCES = denorm.cxx

check_PROGRAMS += test_det test_libm
test_det_SOURCES = test_det.cxx
test_libm_SOURCES = test_libm.cxx


VERROU_FLAG = -O0 -g3 -fno-builtin @vg_test_no_avx512f_flag@
if HAVE_VERROU_FMA_INTRIN
VERROU_FLAG += @vg_test_mfma_flag@ -DUSE_VERROU_FMA
endif
AM_CFLAGS   += $(AM_FLAG_M3264_PRI)  $(VERROU_FLAG)
AM_CXXFLAGS += $(AM_FLAG_M3264_PRI)  -std=c++11 $(VERROU_FLAG)
LDADD  =  -lm
