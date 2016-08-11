#ifndef VERSION_H
#define VERSION_H

#define __VERSION "1.0.0.160808 测试版"
#if defined(Q_OS_WIN)
    #define VERSION __VERSION" (Windows 32-bit)"
#elif defined(Q_OS_LINUX)
    #if defined(Q_PROCESSOR_X86_64)
        #define VERSION __VERSION" (Linux 64-bit)"
    #else
        #define VERSION __VERSION" (Linux 32-bit)"
    #endif
#endif

#endif // VERSION_H
