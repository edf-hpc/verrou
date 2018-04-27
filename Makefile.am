include $(top_srcdir)/Makefile.tool.am

EXTRA_DIST = \
	docs/vr-manual.xml \
	docs/verrou_dd-manpage.xml

pkginclude_HEADERS = verrou.h

#----------------------------------------------------------------------------
# verrou-<platform>
#----------------------------------------------------------------------------

bin_SCRIPTS = verrou_dd
pkgpython_PYTHON = DD.py
install-exec-local:
	mkdir -p ${pkgpythondir}
	touch ${pkgpythondir}/__init__.py
	install -t ${prefix} -m 644 env.sh

noinst_PROGRAMS  = verrou-@VGCONF_ARCH_PRI@-@VGCONF_OS@
if VGCONF_HAVE_PLATFORM_SEC
noinst_PROGRAMS += verrou-@VGCONF_ARCH_SEC@-@VGCONF_OS@
endif

VERROU_SOURCES_COMMON = \
	vr_main.c	\
	vr_exclude.c	\
	vr_error.c	\
	vr_clreq.c	\
	vr_clo.c	\
	backend_verrou/interflop_verrou.cxx

VERROU_FLAG_CXX = $(subst \
	-Wmissing-prototypes,,$(subst \
	-Wstrict-prototypes,,$(subst \
	-Wmissing-parameter-type,,$(subst \
	-Wold-style-declaration,,$(subst \
	-std=gnu99,,$(AM_CFLAGS_@VGCONF_PLATFORM_PRI_CAPS@))))))


# Disable exceptions and RTTI to avoid problems with `__gxx_personality_v0'
# Enable c++11 for std::uint64_t
VERROU_FLAG_CXX += -fno-exceptions -fno-rtti -std=c++11

# We use the test of fma normally use to configure check
if HAVE_FMA_INTRIN
VERROU_FLAG_CXX += -march=native -mfma -DUSE_VERROU_FMA
VERROU_FLAG_C= -march=native -DUSE_VERROU_FMA
endif

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
