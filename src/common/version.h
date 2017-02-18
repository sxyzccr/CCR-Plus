#ifndef VERSION_H
#define VERSION_H

#define YEAR_STRING "2017"
#define DATE_STRING YEAR_STRING "0220"

#if defined(Q_OS_WIN)
#   define PLATFORM_INFO "Windows 32-bit"
#elif defined(Q_OS_LINUX)
#   if defined(Q_PROCESSOR_X86_64)
#       define PLATFORM_INFO "Linux 64-bit"
#   else
#       define PLATFORM_INFO "Linux 32-bit"
#   endif
#endif

#define VERSION_SHORTER "1.1.0"
#define VERSION_SHORT "v" VERSION_SHORTER
#define VERSION_LONG VERSION_SHORT " (" PLATFORM_INFO ")"
#define VERSION_LONGER VERSION_SHORT " " DATE_STRING " (" PLATFORM_INFO ")"

#endif // VERSION_H
