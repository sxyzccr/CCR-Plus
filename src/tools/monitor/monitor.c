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

void run_win()
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
            else printf("进程被阻塞\n"), exit(3);
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

void run_linux()
{
    int mem = (memLim + 5) * (1 << 20), time = timeLim + 1;
    struct rlimit memrlimit = {mem, mem}, timerlimit = {time, time};
    setrlimit(RLIMIT_AS, &memrlimit);
    setrlimit(RLIMIT_CPU, &timerlimit);
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) printf("无法跟踪子进程\n"), exit(1);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    if (execl(exe, exe, NULL) == -1) printf("无法运行程序\n"), exit(1);
}

int getProcessMemory(pid_t pid)        //in kiB
{
    FILE* file;
    char s[256];
    const char* mark = "VmPeak:";
    int res = 0, l = strlen(mark);
    sprintf(s, "/proc/%d/status", pid);
    file = fopen(s, "r");
    if (!file) return -1;
    for (; fgets(s, 255, file);)
    {
        s[strlen(s)] = 0;
        if (!strncmp(s, mark, l))
        {
            sscanf(s + l + 1, "%d", &res);
            break;
        }
    }
    fclose(file);
    return res;
}

void watch_linux()
{
    int status;
    struct rusage usage;
    double usedMem = -1, usedTime;
    for (;;)
    {
        if (wait4(pid, &status, 0, &usage) == -1) printf("运行时错误: 未知\n"), exit(2);

        usedTime = usage.ru_utime.tv_usec / 1000000.0 + usage.ru_utime.tv_sec;
        double mem = getProcessMemory(pid) / 1024.0;
        if (mem > usedMem) usedMem = mem;
        if (usedTime > timeLim) printf("超过时间限制\n"), exit(3);
        if (usedMem > memLim) printf("超过内存限制\n"), exit(4);

        int exitCode = WEXITSTATUS(status);
        if (exitCode && exitCode != 5) //运行时错误
        {
            if (exitCode == SIGXCPU) printf("超过时间限制\n"), exit(3);
            else if (exitCode == SIGSEGV) printf("运行时错误: 段错误\n"), exit(2);
            else if (exitCode == SIGFPE) printf("运行时错误: 浮点异常\n"), exit(2);
            else printf("运行时错误: 未知\n"), exit(2);
        }
        if (WIFEXITED(status))        //正常结束
            printf("时间: %.2lfs 内存: %.2lfMB\n%.6lf\n", usedTime, usedMem, usedTime), exit(0);
        if (WIFSIGNALED(status))    //因为信号而结束
        {
            int sign = WTERMSIG(status);
            if (sign == SIGXCPU) printf("超过时间限制\n"), exit(3);
            else if (sign == SIGSEGV) printf("运行时错误: 段错误\n"), exit(2);
            else if (sign == SIGFPE) printf("运行时错误: 浮点异常\n"), exit(2);
            else if (sign == SIGKILL || sign == SIGABRT) printf("超过内存限制\n"), exit(4);
            else printf("运行时错误: 未知\n"), exit(2);
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
    run_win();
#endif

#ifdef __linux
    pid = fork();
    if (pid < 0) printf("无法创建子进程\n"), exit(1);
    if (!pid) run_linux(); else watch_linux();
#endif
    return 0;
}
//return:    0:N; 1:E; 2:R; 3:T; 4:M;
