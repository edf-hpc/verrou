include $(top_srcdir)/Makefile.tool.am

EXTRA_DIST = \
	docs/vr-manual.xml \
	docs/verrou_dd-manpage.xml \
	docs/verrou_postdd-manpage.xml \
	docs/verrou_plotstat-manpage.xml

pkginclude_HEADERS = verrou.h libVerrouTask/libverrouTask.h

#----------------------------------------------------------------------------
# python tools
#----------------------------------------------------------------------------

PYTHON_REP=pyTools

bin_SCRIPTS = ${PYTHON_REP}/verrou_dd_line ${PYTHON_REP}/verrou_dd_sym ${PYTHON_REP}/genCovBB ${PYTHON_REP}/verrou_plot_stat ${PYTHON_REP}/paraview_script.py ${PYTHON_REP}/verrou_dd_task ${PYTHON_REP}/post_verrou_dd ${PYTHON_REP}/verrou_dd_stdout  ${PYTHON_REP}/verrou_dd_back  ${PYTHON_REP}/verrou_dd_clean libVerrouTask/trace_verrou_task.py ${PYTHON_REP}/find_dd_fail

pkgpython_PYTHON = ${PYTHON_REP}/DD.py ${PYTHON_REP}/dd_config.py ${PYTHON_REP}/DD_stoch.py ${PYTHON_REP}/DD_exec_stat.py ${PYTHON_REP}/convNumLineTool.py ${PYTHON_REP}/post_config.py ${PYTHON_REP}/gen_config.py ${PYTHON_REP}/rounding_tool.py pyWrapper/verrouPyBinding.py

#----------------------------------------------------------------------------
# bash completion script
#----------------------------------------------------------------------------
completion_dir= bash_completion
completion_scripts=${completion_dir}/bash_completion_post_verrou_dd    ${completion_dir}/bash_completion_verrou_dd_clean\
		   ${completion_dir}/bash_completion_verrou_dd_stdout  ${completion_dir}/bash_completion_verrou_dd_task\
		   ${completion_dir}/bash_completion_valgrind          ${completion_dir}/bash_completion_verrou_dd_line\
		   ${completion_dir}/bash_completion_verrou_dd_sym     ${completion_dir}/bash_completion_verrou_plot_stat\
		   ${completion_dir}/bash_completion_verrou_dd_back
completion_install_dir=${prefix}/usr/share/completions

# bash completion code generation
# bash_completion_valgrind and ${completion_dir}/bash_completion_verrou_dd_clean are not generated
${completion_dir}/bash_completion_verrou_dd_back:
	cd ${completion_dir} ; ../${PYTHON_REP}/verrou_dd_back bash_completion
${completion_dir}/bash_completion_verrou_dd_sym:
	cd ${completion_dir} ; ../${PYTHON_REP}/verrou_dd_sym bash_completion
${completion_dir}/bash_completion_verrou_dd_line:
	cd ${completion_dir} ; ../${PYTHON_REP}/verrou_dd_line bash_completion
${completion_dir}/bash_completion_verrou_dd_task:
	cd ${completion_dir} ; ../${PYTHON_REP}/verrou_dd_task bash_completion
${completion_dir}/bash_completion_verrou_dd_stdout:
	cd ${completion_dir} ; ../${PYTHON_REP}/verrou_dd_stdout bash_completion
${completion_dir}/bash_completion_post_verrou_dd:
	cd ${completion_dir} ; ../${PYTHON_REP}/post_verrou_dd bash_completion
${completion_dir}/bash_completion_verrou_plot_stat:
	cd ${completion_dir} ; ../${PYTHON_REP}/verrou_plot_stat bash_completion


#----------------------------------------------------------------------------
# install
#----------------------------------------------------------------------------






install-exec-local:${completion_scripts}
	mkdir -p $(DESTDIR)${pkgpythondir}
	touch $(DESTDIR)${pkgpythondir}/__init__.py
	install -t $(DESTDIR)${prefix} -m 644 env.sh
	install -t $(DESTDIR)${verrouUnbuffereddir} verrouUnbuffered.so
	install -t $(DESTDIR)${verrouCbindingDir} verrouCBinding.so
	install -t $(DESTDIR)${verrouTaskDir} libverrouTask.so
	install -t $(DESTDIR)${verrouExtendStdoutPrecisionDir} verrouExtendStdoutPrecision.so
	mkdir -p $(DESTDIR)${completion_install_dir}
	install -m 644 -t $(DESTDIR)${completion_install_dir} ${completion_scripts}

#----------------------------------------------------------------------------
# verrou-<platform>
#----------------------------------------------------------------------------

noinst_PROGRAMS  = verrou-@VGCONF_ARCH_PRI@-@VGCONF_OS@
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += verrou-@VGCONF_ARCH_SEC@-@VGCONF_OS@
endif


VR_GENERATED_FILE=vr_instrumentOp_checkInst.h vr_instrumentOp_impl.h vr_generated_from_templates.h vr_instrumentOp_impl_generated_verrou_specific.h vr_instrumentOp_impl_generated_all_backends.h
# code generation frontend
vr_main.c: ${VR_GENERATED_FILE}

.PHONY: clean_generated
clean: clean_generated

clean_generated:
	rm -f ${VR_GENERATED_FILE}
	find -name "*.o" -exec rm \{\} \;
	find -name "*.so" -exec rm \{\} \;

vr_generated_from_templates.h: generateBackendInterOperator.py vr_interp_operator_template_cast.h vr_interp_operator_template_1args.h vr_interp_operator_template_2args.h vr_interp_operator_template_3args.h
	./generateBackendInterOperator.py
vr_instrumentOp_checkInst.h: generateOpImpl.py
	./generateOpImpl.py
vr_instrumentOp_impl.h: generateOpImpl.py
	./generateOpImpl.py
vr_instrumentOp_impl_generated_verrou_specific.h: generateInstrumentOp_impl.py
	./generateInstrumentOp_impl.py
vr_instrumentOp_impl_generated_all_backends.h: generateInstrumentOp_impl.py
	./generateInstrumentOp_impl.py

VERROU_SOURCES_COMMON = \
	vr_main.c	\
	vr_exclude.c	\
	vr_error.c	\
	vr_clreq.c	\
	vr_clo.c	\
	vr_include_trace.c \
	vr_exclude_back.c \
	vr_IOMatch_clr.c\
	interflop_backends/backend_mcaquad/common/tinymt64.c\
	interflop_backends/interflop_verrou/interflop_verrou.cxx\
	interflop_backends/backend_checkcancellation/interflop_checkcancellation.cxx\
	interflop_backends/backend_check_float_max/interflop_check_float_max.cxx\
	interflop_backends/backend_checkdenorm/interflop_checkdenorm.cxx\
	interflop_backends/interflop-backend-vprec/interflop_vprec.c\
	interflop_backends/interflop-backend-vprec/common/vprec_tools.c\
	interflop/interflop_stdlib.c\
	interflop/iostream/logger.c\
	interflop/fma/interflop_fma.c\
	interflop/sqrt/interflop_sqrt.c\
	interflop_valgrind_stdlib.c


VERROU_FLAG_CXX = $(subst \
	-Wmissing-prototypes,,$(subst \
	-Wstrict-prototypes,,$(subst \
	-Wmissing-parameter-type,,$(subst \
	-Wold-style-declaration,,$(subst \
	-std=gnu99,,$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@))))))

VERROU_FLAG_C = @vg_test_no_avx512f_flag@ -march=native

# Disable exceptions and RTTI to avoid problems with `__gxx_personality_v0'
# Enable c++11 for std::uint64_t
VERROU_FLAG_CXX += @vg_test_no_avx512f_flag@ -fno-exceptions -fno-rtti -std=c++11 -march=native -ffp-contract=off
VERROU_LD =

# We use the test of fma normally use to configure check
if HAVE_VERROU_FMA_INTRIN
VERROU_FLAG_CXX += @vg_test_mfma_flag@ -DUSE_VERROU_FMA
VERROU_FLAG_C   += @vg_test_mfma_flag@ -DUSE_VERROU_FMA
endif
if HAVE_VERROU_SQRT_INTRIN
VERROU_FLAG_CXX += -DUSE_VERROU_SQRT
VERROU_FLAG_C   += -DUSE_VERROU_SQRT
endif

if HAVE_VERROU_QUADMATH
VERROU_SOURCES_COMMON+= \
	interflop_backends/backend_mcaquad/interflop_mcaquad.c\
	interflop_backends/backend_mcaquad/common/fmaqApprox.c
VERROU_LD       += -lquadmath
VERROU_FLAG_CXX +=  -DUSE_VERROU_QUADMATH
VERROU_FLAG_C   +=  -DUSE_VERROU_QUADMATH

endif

if USE_PROFILING_EXACT
VERROU_FLAG_CXX +=  -DPROFILING_EXACT
VERROU_FLAG_C   +=  -DPROFILING_EXACT
endif

if USE_XOSHIRO
VERROU_FLAG_CXX +=  -DUSE_XOSHIRO
VERROU_FLAG_C   +=  -DUSE_XOSHIRO
endif
if USE_VERROU_NANINF

else
VERROU_FLAG_CXX +=  -DVERROU_IGNORE_NANINF_CHECK
VERROU_FLAG_C   +=  -DVERROU_IGNORE_NANINF_CHECK
endif


if USE_VERROU_DENORM_HACK_FLOAT
VERROU_FLAG_CXX +=  -DVERROU_DENORM_HACKS_FLOAT
VERROU_FLAG_C   +=  -DVERROU_DENORM_HACKS_FLOAT
endif

if USE_VERROU_DENORM_HACK_DOUBLE
VERROU_FLAG_CXX +=  -DVERROU_DENORM_HACKS_DOUBLE
VERROU_FLAG_C   +=  -DVERROU_DENORM_HACKS_DOUBLE
endif

if USE_VERROU_OPTIM
VERROU_FLAG_CXX +=  -DUSE_VERROU_OPTIM
VERROU_FLAG_C   +=  -DUSE_VERROU_OPTIM
endif

VERROU_FLAG_C +=-DVERROU_NUM_NEARNESS=@VERROU_NUM_NEARNESS@
VERROU_FLAG_CXX +=-DVERROU_NUM_NEARNESS=@VERROU_NUM_NEARNESS@


VERROU_FLAG_C +=-DVERROU_DET_HASH=vr_@vg_cv_verrou_det_hash@_hash
VERROU_FLAG_CXX +=-DVERROU_DET_HASH=vr_@vg_cv_verrou_det_hash@_hash

VERROU_FLAG_C   += -DINTERFLOP_STATIC_INTERFACE_ENABLED
VERROU_FLAG_CXX += -DINTERFLOP_STATIC_INTERFACE_ENABLED

verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_SOURCES      = \
	$(VERROU_SOURCES_COMMON)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)\
	$(VERROU_FLAG_C)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CXXFLAGS     = \
	$(VERROU_FLAG_CXX)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_PRI_CAPS@)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_PRI_CAPS@)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDFLAGS      = \
	$(TOOL_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)
verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LINK = \
	$(top_builddir)/coregrind/link_tool_exe_@VGCONF_OS@ \
	@VALT_LOAD_ADDRESS_PRI@ \
	$(LINK) \
	$(verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_CFLAGS) \
	$(verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_LDFLAGS)





if VGCONF_HAVE_PLATFORM_SEC
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_SOURCES      = \
	$(VERROU_SOURCES_COMMON)
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CPPFLAGS     = \
	$(AM_CPPFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CFLAGS       = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CXXFLAGS     = \
	$(AM_CFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)     \
	-fno-exceptions -fno-rtti
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_DEPENDENCIES = \
	$(TOOL_DEPENDENCIES_@VGCONF_PLATFORM_SEC_CAPS@)
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDADD        = \
	$(TOOL_LDADD_@VGCONF_PLATFORM_SEC_CAPS@)    \
	-lm -lc
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDFLAGS      = \
	$(TOOL_LDFLAGS_@VGCONF_PLATFORM_SEC_CAPS@)
verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LINK = \
	$(top_builddir)/coregrind/link_tool_exe_@VGCONF_OS@ \
	@VALT_LOAD_ADDRESS_SEC@ \
	$(LINK) \
	$(verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_CFLAGS) \
	$(verrou_@VGCONF_ARCH_SEC@_@VGCONF_OS@_LDFLAGS)
endif



#----------------------------------------------------------------------------
# vgpreload_verrou-<platform>.so
#----------------------------------------------------------------------------

noinst_PROGRAMS += vgpreload_verrou-@VGCONF_ARCH_PRI@-@VGCONF_OS@.so
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += vgpreload_verrou-@VGCONF_ARCH_SEC@-@VGCONF_OS@.so
endif

if VGCONF_OS_IS_DARWIN
noinst_DSYMS = $(noinst_PROGRAMS)
endif

VERROUINTERLIBM_SOURCES_COMMON = Interlibmath/interlibmath.cxx Interlibmath/stacktrace.cxx  interflop_backends/backend_mcaquad/common/tinymt64.c

vgpreload_verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_SOURCES      = \
	$(VERROUINTERLIBM_SOURCES_COMMON)

vgpreload_verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_CFLAGS     = \
       $(AM_CFLAGS@VGCONF_PLATFORM_PRI_CAPS@) -fPIC -DLIBMATHINTERP $(VERROU_FLAG_C) -DVGA_@VGCONF_ARCH_PRI@ -Wno-missing-declarations -I../include -fno-builtin @vg_test_no_avx512f_flag@

vgpreload_verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_CXXFLAGS     = \
$(AM_CXXFLAGS@VGCONF_PLATFORM_PRI_CAPS@) -fPIC -DLIBMATHINTERP $(VERROU_FLAG_CXX) -DVGA_@VGCONF_ARCH_PRI@ -Wno-missing-declarations -I../include -fno-builtin @vg_test_no_avx512f_flag@

vgpreload_verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_LDFLAGS      = \
	-static-libstdc++ -lm \
	$(subst -nodefaultlibs,,$(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@))
vgpreload_verrou_@VGCONF_ARCH_PRI@_@VGCONF_OS@_so_LDADD  =  -ldl $(VERROU_LD)

#----------------------------------------------------------------------------
# verrouUnbuffered.so
#----------------------------------------------------------------------------
verrouUnbuffereddir = $(pkglibdir)
noinst_PROGRAMS += verrouUnbuffered.so

VERROUUNBUFFERED_SOURCES_COMMON = unbufferPrintf.c

verrouUnbuffered_so_SOURCES      = \
	$(VERROUUNBUFFERED_SOURCES_COMMON)
verrouUnbuffered_so_CPPFLAGS     = \
	$(subst -Wstrict-prototypes,,$(AM_CPPFLAGS@VGCONF_PLATFORM_PRI_CAPS@)) $(AM_CFLAGS_PSO_@VGCONF_PLATFORM_PRI_CAPS@)
verrouUnbuffered_so_LDFLAGS      = $(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)


#----------------------------------------------------------------------------
# verrouExtendStdoutPrecision.so
#----------------------------------------------------------------------------
verrouExtendStdoutPrecisionDir = $(pkglibdir)
noinst_PROGRAMS += verrouExtendStdoutPrecision.so

VERROUEXTENDSTDOUPRECISION_SOURCES_COMMON = extendStdoutPrecision.cxx

verrouExtendStdoutPrecision_so_SOURCES      = \
	$(VERROUEXTENDSTDOUPRECISION_SOURCES_COMMON)
verrouExtendStdoutPrecision_so_CPPFLAGS     = \
	$(subst -Wstrict-prototypes,,\
	$(subst -Wmissing-declarations,,\
	$(subst -Wmissing-parameter-type,,\
	$(subst -Wold-style-declaration,,\
	$(subst -Wmissing-prototypes,,$(AM_CPPFLAGS@VGCONF_PLATFORM_PRI_CAPS@) $(AM_CFLAGS_PSO_@VGCONF_PLATFORM_PRI_CAPS@))))))
verrouExtendStdoutPrecision_so_LDFLAGS      =  $(subst -nodefaultlibs,,$(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@))
#Suppression of -nodefaultlibs is required to avoid error with program written without C++
verrouExtendStdoutPrecision_so_LDADD      = -ldl

#----------------------------------------------------------------------------
# verrouCBinding.so
#----------------------------------------------------------------------------
verrouCbindingDir = $(pkglibdir)
noinst_PROGRAMS += verrouCBinding.so


VERROUCBINDING_SOURCES_COMMON = pyWrapper/verrouCBinding.c

verrouCBinding_so_SOURCES      = $(VERROUCBINDING_SOURCES_COMMON)
verrouCBinding_so_CPPFLAGS     = \
	$(AM_CPPFLAGS@VGCONF_PLATFORM_PRI_CAPS@) $(AM_CFLAGS_PSO_@VGCONF_PLATFORM_PRI_CAPS@)
verrouCBinding_so_LDFLAGS      = $(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@)


#----------------------------------------------------------------------------
# libverrouTask.so
#----------------------------------------------------------------------------
verrouTaskDir = $(pkglibdir)
noinst_PROGRAMS += libverrouTask.so
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += libverrouTask.so
endif

PRELOADTASK1 = $(subst -nodefaultlibs,,$(PRELOAD_LDFLAGS_@VGCONF_PLATFORM_PRI_CAPS@))
PRELOADTASK = $(subst -m64,,$(PRELOADTASK1))

VERROUTASK_SOURCES_COMMON = libVerrouTask/libverrouTask.cxx
libverrouTask_so_SOURCES      = $(VERROUTASK_SOURCES_COMMON)
libverrouTask_so_CPPFLAGS     = \
	$(subst -Wstrict-prototypes,,\
	$(subst -Wmissing-declarations,,\
	$(subst -Wmissing-parameter-type,,\
	$(subst -Wold-style-declaration,,\
	$(subst -Wmissing-prototypes,,$(AM_CPPFLAGS@VGCONF_PLATFORM_PRI_CAPS@) $(AM_CFLAGS_PSO_@VGCONF_PLATFORM_PRI_CAPS@))))))\
	-I../include
libverrouTask_so_LDFLAGS      = -static-libstdc++ $(PRELOADTASK)
