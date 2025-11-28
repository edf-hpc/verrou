#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#define String HChar *

typedef struct vki_timeval timeval_t;
typedef struct vki_timezone timezone_t;

VgFile *_interflop_stderr(void);

void *_interflop_valgrind_malloc(SizeT size);
VgFile *_interflop_valgrind_fopen(const String pathname, const String mode,
                                  Int *error);
Int _interflop_valgrind_strcmp(const String s1, const String s2);
Int _interflop_valgrind_strcasecmp(const String s1, const String s2);
/* Do not follow libc API, use error pointer to pass errno result instead */
/* error = 0 if success */
Long _interflop_valgrind_strtol(const String nptr, String *endptr, Int *error);
double _interflop_valgrind_strtod(const String nptr, String *endptr,
                                  Int *error);
String _interflop_valgrind_getenv(const String name);
Int _interflop_valgrind_fprintf(VgFile *stream, const String format, ...);

String _interflop_valgrind_strcpy(String dest, const String src);
String _interflop_valgrind_strncpy(String dest, const String src, SizeT size);

/* VG_(fclose) does not return error code */
Int _interflop_valgrind_fclose(VgFile *stream);

Int _interflop_valgrind_gettid(void);
const String _interflop_valgrind_strerror(Int error);
Int _interflop_valgrind_sprintf(String str, const String format, ...);

/* Valgrind does not have vwarnx equivalent, so just use fprintf */
void _interflop_valgrind_vwarnx(const String fmt, va_list args);

Int _interflop_valgrind_vfprintf(VgFile *stream, const String format,
                                 va_list ap);
void _interflop_valgrind_exit(Int status);
String _interflop_valgrind_strtok_r(String str, const String delim,
                                    String *saveptr);

/* Valgrind does not have function to read file so we have to reimplement it */
String _interflop_valgrind_fgets(String s, Int size, VgFile *stream);

void _interflop_valgrind_free(void *ptr);
void *_interflop_valgrind_calloc(SizeT nmemb, SizeT size);
Int _interflop_valgrind_gettimeofday(timeval_t *tv, timezone_t *tz);
