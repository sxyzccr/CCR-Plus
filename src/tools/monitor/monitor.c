#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double timeLim, memLim;
char* exe;

#ifdef __WIN32
#include <windows.h>
#include <psapi.h>

STARTUPINFO si;
PROCESS_INFORMATION pi;
PROCESS_MEMORY_COUNTERS pmc;
FILETIME ct, et, kt, ut;
SYSTEMTIME _kt, _ut;

void onProcessFinished()
{
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

void run()
{
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    if (!CreateProcess(0, (LPTSTR)exe, 0, 0, FALSE, CREATE_NO_WINDOW, 0, 0, &si, &pi))
        printf("无法运行程序: %u\n", (unsigned int)GetLastError()), exit(1);

    int beginTime = clock();
    for (;;)
    {
        if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) break;

        GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc));
        if (pmc.PeakPagefileUsage > memLim * (1<<20))
        {
            TerminateProcess(pi.hProcess, 0);
            onProcessFinished();
            printf("超过内存限制\n");
            exit(4);
        }

        GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
        FileTimeToSystemTime(&kt, &_kt);
        FileTimeToSystemTime(&ut, &_ut);
        double userTime = _ut.wHour * 3600 + _ut.wMinute * 60 + _ut.wSecond + _ut.wMilliseconds / 1000.0;
        double kernelTime = _kt.wHour * 3600 + _kt.wMinute * 60 + _kt.wSecond + _kt.wMilliseconds / 1000.0;
        double blockTime = (double)(clock() - beginTime) / CLOCKS_PER_SEC - userTime - kernelTime;

        if (userTime > timeLim || kernelTime > timeLim || blockTime > 1.5)
        {
            TerminateProcess(pi.hProcess, 0);
            onProcessFinished();
            if (userTime > timeLim) printf("超过时间限制\n"), exit(3);
            else if (kernelTime > timeLim) printf("系统 CPU 时间过长\n"), exit(3);
            else printf("进程被阻塞\n"), exit(2);
        }

        Sleep(10);
    }

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (exitCode && exitCode != STILL_ACTIVE)
    {
        onProcessFinished();
        printf("运行时错误: %u\n", (unsigned int)exitCode);
        exit(2);
    }

    GetProcessMemoryInfo(pi.hProcess, &pmc, sizeof(pmc));
    double usedMemory = pmc.PeakPagefileUsage / 1024.0 / 1024.0;

    GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut);
    FileTimeToSystemTime(&ut, &_ut);
    double usedTime = _ut.wHour * 3600 + _ut.wMinute * 60 + _ut.wSecond + _ut.wMilliseconds / 1000.0;

    onProcessFinished();

    if (usedMemory > memLim) printf("超过内存限制\n"), exit(4);
    if (usedTime > timeLim) printf("超过时间限制\n"), exit(3);

    printf("时间: %.2lfs 内存: %.2lfMB\n%.6lf\n", usedTime, usedMemory, usedTime);
    exit(0);
}
#endif

#ifdef __linux
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ptrace.h>
#include <sys/resource.h>

pid_t pid;
struct timespec beginTime, nowTime;
unsigned int memPeak = 0; // in kiB
double userTime, systemTime, blockTime; // in second
int watching = 0;

void run()
{
    unsigned long long mem = (memLim + 5) * (1ll << 20), time = timeLim + 1;
    struct rlimit memrlimit = {mem, mem}, timerlimit = {time, time + 1};
    setrlimit(RLIMIT_AS, &memrlimit);
    setrlimit(RLIMIT_CPU, &timerlimit);

    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) printf("无法跟踪子进程\n"), exit(1);
    if (execl(exe, exe, NULL) == -1) printf("无法运行程序\n"), exit(1);
}

unsigned int get_process_memory(pid_t pid)  // in kiB
{
    FILE* file;
    char buf[256], filename[256];
    const char* mark = "VmPeak:";
    unsigned int mem = 0, len = strlen(mark);

    sprintf(filename, "/proc/%d/status", pid);
    file = fopen(filename, "r");
    if (!file) return 0;

    for (; fgets(buf, 255, file);)
    {
        buf[strlen(buf)] = '\0';
        if (!strncmp(buf, mark, len))
        {
            sscanf(buf + len + 1, "%d", &mem);
            break;
        }
    }
    fclose(file);

    return mem;
}

int get_cpu_time(pid_t pid, double* utime, double* stime) // in second
{
    FILE *file;
    char buf[2048], *open_paren, *close_paren, filename[256];
    unsigned int ut, st;

    sprintf(filename, "/proc/%d/stat", pid);
    file = fopen(filename, "r");
    if (!file) return 1;
    fgets(buf, 2048, file);
    fclose(file);

    // Split at first '(' and last ')' to get process name.
    open_paren = strchr(buf, '(');
    close_paren = strrchr(buf, ')');
    if (!open_paren || !close_paren) return 1;

    // Scan rest of string.
    sscanf(close_paren + 1, " %*c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %u %u", &ut, &st);

    *utime = (double)ut / sysconf(_SC_CLK_TCK);
    *stime = (double)st / sysconf(_SC_CLK_TCK);

    return 0;
}

double get_elapsed_time() // in second
{
    clock_gettime(CLOCK_REALTIME, &nowTime);
    return nowTime.tv_sec + nowTime.tv_nsec / 1e9 - beginTime.tv_sec - beginTime.tv_nsec / 1e9;
}

void watch()
{
    if (watching) return;
    watching = 1;

    // 超内存
    unsigned int mem = get_process_memory(pid);
    if (mem > memPeak) memPeak = mem;
    if (memPeak > memLim * 1024) printf("超过内存限制\n"), exit(4);

    // 超时
    get_cpu_time(pid, &userTime, &systemTime);
    blockTime = get_elapsed_time() - userTime - systemTime;

    if (userTime > timeLim) printf("超过时间限制\n"), exit(3);
    if (systemTime > timeLim) printf("系统 CPU 时间过长\n"), exit(3);
    if (blockTime > 1.5) printf("进程被阻塞\n"), exit(2);

    watching = 0;
}

void watch_rusage(struct rusage* usage)
{
    if (watching) return;
    watching = 1;

     // 超内存
    unsigned int mem = get_process_memory(pid);
    if (mem > memPeak) memPeak = mem;
    if (memPeak > memLim * 1024) printf("超过内存限制\n"), exit(4);

    // 超时
    userTime = usage->ru_utime.tv_usec / 1000000.0 + usage->ru_utime.tv_sec;
    systemTime = usage->ru_stime.tv_usec / 1000000.0 + usage->ru_stime.tv_sec;
    blockTime = get_elapsed_time() - userTime - systemTime;

    if (userTime > timeLim) printf("超过时间限制\n"), exit(3);
    if (systemTime > timeLim) printf("系统 CPU 时间过长\n"), exit(3);
    if (blockTime > 1.5) printf("进程被阻塞\n"), exit(2);

    watching = 0;
}

void trace()
{
    int status;
    struct rusage usage;
    clock_gettime(CLOCK_REALTIME, &beginTime);

    // Watch the child process state per 0.1 sec
    struct itimerval tick;
    tick.it_value.tv_sec = 0;
    tick.it_value.tv_usec = 100000;
    tick.it_interval.tv_sec = 0;
    tick.it_interval.tv_usec = 100000;
    setitimer(ITIMER_REAL, &tick, NULL);
    signal(SIGALRM, watch);

    // Trace child process
    for (;;)
    {
        if (wait4(pid, &status, 0, &usage) == -1) printf("运行时错误: 未知\n"), exit(2);

        // 检测内存、时间
        watch_rusage(&usage);

        // 运行时错误
        int exitCode = WEXITSTATUS(status);
        if (exitCode && exitCode != SIGTRAP)
        {
            if (WIFEXITED(status)) printf("运行时错误: %d\n", exitCode), exit(2);
            else if (exitCode == SIGXCPU) printf("超过时间限制\n"), exit(3);
            else if (exitCode == SIGSEGV) printf("运行时错误: 段错误\n"), exit(2);
            else if (exitCode == SIGFPE) printf("运行时错误: 浮点异常\n"), exit(2);
            else if (exitCode == SIGKILL || exitCode == SIGABRT) printf("超过内存限制\n"), exit(4);
            else printf("运行时错误: %d\n", exitCode), exit(2);
        }

        // 正常结束
        if (WIFEXITED(status))
            printf("时间: %.2lfs 内存: %.2lfMB\n%.6lf\n", userTime, memPeak / 1024.0, userTime), exit(0);

        // 因为信号而结束
        if (WIFSIGNALED(status))
        {
            int sign = WTERMSIG(status);
            if (sign == SIGXCPU) printf("超过时间限制\n"), exit(3);
            else if (sign == SIGSEGV) printf("运行时错误: 段错误\n"), exit(2);
            else if (sign == SIGFPE) printf("运行时错误: 浮点异常\n"), exit(2);
            else if (sign == SIGKILL || sign == SIGABRT) printf("超过内存限制\n"), exit(4);
            else printf("运行时错误: %d\n", sign), exit(2);
        }

        if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1) printf("无法跟踪子进程\n"), exit(1);
    }
}
#endif

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
#ifdef __WIN32
        fprintf(stderr, "Usage:\nmonitior.exe EXE_FILE TIME_LIM MEM_LIM\n");
#else
        fprintf(stderr, "Usage:\n./monitior EXE_FILE TIME_LIM MEM_LIM\n");
#endif
        return    0;
    }
    exe = argv[1];
    sscanf(argv[2], "%lf", &timeLim);
    sscanf(argv[3], "%lf", &memLim);

#ifdef __WIN32
    run();
#endif

#ifdef __linux
    pid = fork();
    if (pid < 0) printf("无法创建子进程\n"), exit(1);
    if (!pid) run(); else trace();
#endif
    return 0;
}
//return:    0:N; 1:E; 2:R; 3:T; 4:M;
