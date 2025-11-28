

#include "interflop_valgrind_stdlib.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcfile.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_libcproc.h"
#include "pub_tool_mallocfree.h"
#include "valgrind.h"

#define VGFILE_BUFSIZE 8192

struct _VgFile {
  HChar buf[VGFILE_BUFSIZE];
  UInt num_chars; // number of characters in buf
  Int fd;         // file descriptor to write to
};

VgFile *_interflop_stderr(void) {
  VgFile *fp = (VgFile *)VG_(malloc)("fdopen", sizeof(struct _VgFile));
  fp->fd = 2;
  fp->num_chars = 0;
  return fp;
}

void *_interflop_valgrind_malloc(SizeT size) { return VG_(malloc)("", size); }
VgFile *_interflop_valgrind_fopen(const String pathname, const String mode,
                                  Int *error) {

  /* This function does not check for rw|rw+ case */
  /* Neither other flags x,F,e and c that are extensions */
  //   ┌─────────────┬───────────────────────────────┐
  //   │fopen() mode │ open() flags                  │
  //   ├─────────────┼───────────────────────────────┤
  //   │     r       │ O_RDONLY                      │
  //   ├─────────────┼───────────────────────────────┤
  //   │     w       │ O_WRONLY | O_CREAT | O_TRUNC  │
  //   ├─────────────┼───────────────────────────────┤
  //   │     a       │ O_WRONLY | O_CREAT | O_APPEND │
  //   ├─────────────┼───────────────────────────────┤
  //   │     r+      │ O_RDWR                        │
  //   ├─────────────┼───────────────────────────────┤
  //   │     w+      │ O_RDWR | O_CREAT | O_TRUNC    │
  //   ├─────────────┼───────────────────────────────┤
  //   │     a+      │ O_RDWR | O_CREAT | O_APPEND   │
  //   └─────────────┴───────────────────────────────┘

  Int flags = 0;
  if (VG_(strcmp)(mode, "r")) {
    flags = VKI_O_RDONLY;
  } else if (VG_(strcmp)(mode, "r+")) {
    flags = VKI_O_RDWR;
  } else if (VG_(strcmp)(mode, "w")) {
    flags = VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC;
  } else if (VG_(strcmp)(mode, "w+")) {
    flags = VKI_O_RDWR | VKI_O_CREAT | VKI_O_TRUNC;
  } else if (VG_(strcmp)(mode, "a")) {
    flags = VKI_O_WRONLY | VKI_O_CREAT | VKI_O_APPEND;
  } else if (VG_(strcmp(mode, "a+"))) {
    flags = VKI_O_RDWR | VKI_O_CREAT | VKI_O_APPEND;
  }

  // flags = mode linux
  // mode = permission
  Int permissions = VKI_S_IRUSR | VKI_S_IWUSR | VKI_S_IRGRP | VKI_S_IROTH;

  return VG_(fopen)(pathname, flags, permissions);
}

Int _interflop_valgrind_strcmp(const String s1, const String s2) {
  return VG_(strcmp)(s1, s2);
}

Int _interflop_valgrind_strcasecmp(const String s1, const String s2) {
  return VG_(strcasecmp)(s1, s2);
}

/* Do not follow libc API, use error pointer to pass errno result instead */
/* error = 0 if success */
Long _interflop_valgrind_strtol(const String nptr, String *endptr, Int *error) {
  // VG_(strtoll10) returns 0 if no number could be converted,
  // and 'endptr' is set to the start of the string.
  *error = 0;
  long val = VG_(strtoll10)(nptr, endptr);
  if (val == 0 && *endptr == nptr) {
    *error = 1;
  }
  return val;
}

/* Do not follow libc API, use error pointer to pass errno result instead */
/* error = 0 if success */
double _interflop_valgrind_strtod(const String nptr, String *endptr,
                                  Int *error) {
  // VG_(strtod) returns 0 if no number could be converted,
  // and 'endptr' is set to the start of the string.
  *error = 0;
  double val = VG_(strtod)(nptr, endptr);
  if (val == 0 && *endptr == nptr) {
    *error = 1;
  }
  return val;
}

String _interflop_valgrind_getenv(const String name) {
  return VG_(getenv)(name);
}

static void __fflush(VgFile *stream) {
  VG_(write)(stream->fd, stream->buf, stream->num_chars);
  stream->num_chars = 0;
}

Int _interflop_valgrind_fprintf(VgFile *stream, const String format, ...) {
  va_list ap;
  va_start(ap, format);
  UInt res = VG_(vfprintf)(stream, format, ap);
  __fflush(stream);
  va_end(ap);
  return res;
}

String _interflop_valgrind_strcpy(String dest, const String src) {
  return VG_(strcpy)(dest, src);
}

String _interflop_valgrind_strncpy(String dest, const String src, SizeT size) {
   return VG_(strncpy)(dest, src, size);
}


/* VG_(fclose) does not return error code */
Int _interflop_valgrind_fclose(VgFile *stream) {
  VG_(fclose)(stream);
  return 0;
}

Int _interflop_valgrind_gettid(void) { return VG_(gettid)(); }

const String _interflop_valgrind_strerror(Int error) {
  return "internal error\n";
}

Int _interflop_valgrind_sprintf(String str, const String format, ...) {
  va_list ap;
  va_start(ap, format);
  UInt res = VG_(sprintf)(str, format, ap);
  va_end(ap);
  return res;
}

/* Valgrind does not have vwarnx equivalent, so just use fprintf */
void _interflop_valgrind_vwarnx(const String fmt, va_list args) {
  VG_(vprintf)(fmt, args);
}

Int _interflop_valgrind_vfprintf(VgFile *stream, const String format,
                                 va_list ap) {
  Int res = VG_(vfprintf)(stream, format, ap);
  __fflush(stream);
  return res;
}

void _interflop_valgrind_exit(Int status) { VG_(exit)(status); }

String _interflop_valgrind_strtok_r(String str, const String delim,
                                    String *saveptr) {
  return VG_(strtok_r)(str, delim, saveptr);
}

/* Valgrind does not have function to read file so we have to reimplement it */
String _interflop_valgrind_fgets(String s, Int size, VgFile *stream) {
  // fgets() reads in at most one less than size characters from stream and
  // stores them into the buffer pointed to by s. Reading stops after an EOF or
  // a newline. If a newline is read, it is stored into the buffer. A
  // terminating null byte ('\0') is stored after the last character in the
  // buffer.
  if (size <= 0) {
    return NULL;
  }
  if (size == 1) {
    s[0] = '\0';
    return s;
  }

  Int fd = stream->fd;
  Int _read = 0;
  Int _nb_read = 0;
  HChar c;
  _read = VG_(read)(fd, (void *)&c, 1);
  while (_read != 0 && c != '\n' && _nb_read < size - 2) {
    s[_nb_read] = c;
    _nb_read++;
    _read = VG_(read)(fd, (void *)&c, 1);
  }

  s[_nb_read] = c;
  s[_nb_read + 1] = '\0';

  if (_read == 0) {
    return NULL;
  }

  return s;
}

void _interflop_valgrind_free(void *ptr) { VG_(free)(ptr); }

void *_interflop_valgrind_calloc(SizeT nmemb, SizeT size) {
  return VG_(calloc)("", nmemb, size);
}

Int _interflop_valgrind_gettimeofday(timeval_t *tv, timezone_t *tz) {
  return VG_(gettimeofday)(tv, tz);
}
