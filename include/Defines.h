#ifndef DEFINES_H
#define DEFINES_H

#ifndef LITTLEENDIAN
#define LITTLEENDIAN 0
#endif

#ifndef BIGENDIAN
#define BIGENDIAN 1
#endif

#if !defined(ENDIAN)
#define ENDIAN LITTLEENDIAN
#endif

#define PLATFORM_WINDOWS 0
#define PLATFORM_UNIX    1
#define PLATFORM_APPLE   2
#define PLATFORM_INTEL   3

// must be first (win 64 also define WIN32)
#if defined( _WIN64 )
#  define PLATFORM PLATFORM_WINDOWS
#elif defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM PLATFORM_WINDOWS
#elif defined( __APPLE_CC__ )
#  define PLATFORM PLATFORM_APPLE
#elif defined( __INTEL_COMPILER )
#  define PLATFORM PLATFORM_INTEL
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU       1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3

#ifdef _MSC_VER
#  define COMPILER COMPILER_MICROSOFT
#elif defined( __BORLANDC__ )
#  define COMPILER COMPILER_BORLAND
#elif defined( __INTEL_COMPILER )
#  define COMPILER COMPILER_INTEL
#elif defined( __GNUC__ )
#  define COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if COMPILER == COMPILER_MICROSOFT
#  pragma warning( disable : 4267 ) // conversion from 'size_t' to 'int', possible loss of data
#  pragma warning( disable : 4786 ) // identifier was truncated to '255' characters in the debug information
#  pragma warning( disable : 4018 ) // Konflikt zwischen 'signed' und 'unsigned'
#  ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_SECURE_NO_DEPRECATE
#  define _CRT_SECURE_NO_DEPRECATE
#  endif
#endif

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F,V)
#endif //COMPILER == COMPILER_GNU

#if COMPILER != COMPILER_MICROSOFT
typedef uint16      WORD;
typedef uint32      DWORD;
#endif //COMPILER

#include <iostream>
#include <conio.h>
#include <set>
#include <deque>
#include <vector>
#include <cassert>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <map>

#if PLATFORM == PLATFORM_WINDOWS 
#include "direct.h"
#include "./dirent.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace std;

typedef signed __int8  int8;
typedef signed __int16 int16;
typedef signed __int32 int32;
typedef signed __int64 int64;

typedef unsigned __int8     uint8;
typedef unsigned __int16    uint16;
typedef unsigned __int32    uint32;
typedef unsigned __int64    uint64;

#if COMPILER == COMPILER_MICROSOFT
#include <float.h>
#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define sscanf sscanf_s
#define atoll __atoi64
#define vsnprintf _vsnprintf
#define strdup _strdup
#define finite(X) _finite(X)
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"
#endif

inline float finiteAlways(float f) { return finite(f) ? f : 0.0f; }

#define atol(a) strtoul( a, NULL, 10)

#define STRINGIZE(a) #a

#define MAX_QUERY_LEN       32*1024
#define MAX_DBCS            236
#define MAX_LOCALE          12

#define _VERSION            "2.1.0"
#define _CLIENT_VERSION     "3.2.2.10505"
#define _YEAR               "2008-2009"
#define _CONFIGFILE         "DataTrap.cfg"
#define _LOGFILE            "DataTrap.log"

#endif
