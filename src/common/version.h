#ifndef VERSION_H
#define VERSION_H

#define YEAR_STRING "2017"
#define DATE_STRING YEAR_STRING "0220"

#include "qglobal.h"

#if defined(Q_OS_WIN)
#   if defined(Q_PROCESSOR_X86_64)
#       define PLATFORM_INFO "Windows x64"
#   elif defined(Q_PROCESSOR_X86_32)
#       define PLATFORM_INFO "Windows x86"
#   endif
#elif defined(Q_OS_LINUX)
#   if defined(Q_PROCESSOR_X86_64)
#       define PLATFORM_INFO "Linux x86-64"
#   elif defined(Q_PROCESSOR_X86_32)
#       define PLATFORM_INFO "Linux x86-32"
#   elif defined(Q_PROCESSOR_ARM_64)
#       define PLATFORM_INFO "Linux aarch64"
#   elif defined(Q_PROCESSOR_ARM_32)
#       define PLATFORM_INFO "Linux armhf"
#   elif defined(Q_PROCESSOR_RISCV)
#       #if defined(Q_PROCESSOR_BITS) && Q_PROCESSOR_BITS == 64
#           define PLATFORM_INFO "Linux riscv 64-bit"
#       #elif defined(Q_PROCESSOR_BITS) && Q_PROCESSOR_BITS == 32
#           define PLATFORM_INFO "Linux riscv 32-bit"
#       #endif
#   endif
#endif

#define VERSION_SHORTER "1.1.0"
#define VERSION_SHORT "v" VERSION_SHORTER
#define VERSION_LONG VERSION_SHORT " (" PLATFORM_INFO ")"
#define VERSION_LONGER VERSION_SHORT " " DATE_STRING " (" PLATFORM_INFO ")"

#endif // VERSION_H
