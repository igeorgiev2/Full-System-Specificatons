
#include <iostream>
#include <cstring>
#include <string>

using namespace std;

#if defined __unix || defined __unix__ || defined __linux__ || defined __FreeBSD__
    #include <unistd.h>
    #include <sys/wait.h>
    #include <cpuid.h>

    int Get_System_Info_Unix() {
        system("uname -a");
        system("lshw -short");
        char CPUBrandString[0x40];
        unsigned int CPUInfo[4] = {0,0,0,0};

        __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
        unsigned int nExIds = CPUInfo[0];

        memset(CPUBrandString, 0, sizeof(CPUBrandString));

        for (unsigned int i = 0x80000000; i <= nExIds; ++i)
        {
        __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);

        if (i == 0x80000002)
        memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
        memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
        memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }

        cout << "CPU Type " << CPUBrandString << endl;
        system("lscpu");
        system("lshw -class display");
        system("nvidia-smi");
        system("glxinfo | grep OpenGL");
        return 0;
    }

    unsigned long long getTotalSystemMemoryUnix() {
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return (((pages * page_size)/1024)/1024);
    }
    
    char *Get_OS_Info(char *cmd) {
        int buff_size = 32;
        char *buff = new char[buff_size];
        char *ret = NULL;
        string str = "";
        int fd[2];
        int old_fd[3];
        pipe(fd);
        old_fd[0] = dup(STDIN_FILENO);
        old_fd[1] = dup(STDOUT_FILENO);
        old_fd[2] = dup(STDERR_FILENO);
        int pid = fork();
        switch(pid) {
            case 0:
                close(fd[0]);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
                dup2(fd[1], STDOUT_FILENO);
                dup2(fd[1], STDERR_FILENO);
                system(cmd);
                //execlp((const char*)cmd, cmd,0);
                close (fd[1]);
                exit(0);
            break;
            case -1:
                cerr << "GetSystemOutput/fork() error\n" << endl;
                exit(1);
            default:
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                int rc = 1;
                while (rc > 0) {
                    rc = read(fd[0], buff, buff_size);
                    str.append(buff, rc);
                    //memset(buff, 0, buff_size);
                }
                ret = new char [strlen((char*)str.c_str())];
                strcpy(ret, (char*)str.c_str());
                waitpid(pid, NULL, 0);
                close(fd[0]);
        }
        dup2(STDIN_FILENO, old_fd[0]);
        dup2(STDOUT_FILENO, old_fd[1]);
        dup2(STDERR_FILENO, old_fd[2]);
        return ret;
    }
#endif

#if defined _WIN32 || defined _WIN64
    #include <windows.h>
    #include <intrin.h>

    int Get_System_Info_Windows() {
        int CPUInfo[4] = {-1};
        unsigned   nExIds, i =  0;
        char CPUBrandString[0x40];
        // Get the information associated with each extended ID.
        __cpuid(CPUInfo, 0x80000000);
        nExIds = CPUInfo[0];
        for (i=0x80000000; i<=nExIds; ++i)
        {
            __cpuid(CPUInfo, i);
            // Interpret CPU brand string
            if  (i == 0x80000002)
                memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
            else if  (i == 0x80000003)
                memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
            else if  (i == 0x80000004)
                memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
        //string includes manufacturer, model and clockspeed
        cout << "CPU Type " << CPUBrandString << endl;


        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cout << "Number of Cores: " << sysInfo.dwNumberOfProcessors << endl;

        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof (statex);
        GlobalMemoryStatusEx(&statex);
        cout << "Total System Memory: " << ((statex.ullTotalPhys/1024)/1024) << " Mega Bytes" << endl;
        system("wmic path win32_VideoController get name");
        system("SYSTEMINFO");    
        return 0;
    }

/*
    unsigned long long getTotalSystemMemoryWindows() {
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
    }
*/
#endif

string getOsName() {
    #ifdef _WIN64
    return "Windows 64-bit";
    #elif _WIN32
    return "Windows 32-bit";
    #elif __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __linux__
    return "Linux";
    #elif __FreeBSD__
    return "FreeBSD";
    #elif __unix || __unix__
    return "Unix";
    #else
    return "Other";
    #endif
}



string cpp_version() {
    if (__cplusplus == 201703L) return "C++17";
    else if (__cplusplus == 201402L) return "C++14";
    else if (__cplusplus == 201103L) return "C++11";
    else if (__cplusplus == 199711L) return "C++98";
    else return "pre-standard C++";
}

string getCompilerName() {
    #ifdef _MSC_VER
    return "Microsoft Visual C++ (internal version " + to_string(_MSC_VER) + ")";
    #elif __clang__
        string name = "clang ";
        string version = __clang_version__;
    return name + version;
    #elif __EMSCRIPTEN__
    return "emscripten";
    #elif __MINGW32__
    return "MinGW 32";
    #elif __MINGW64__
    return "MinGW 64bit";
    #elif __GNUC__
        string name = "gcc ";
        string version = to_string(__GNUC__) + "." + to_string(__GNUC_MINOR__) + "." + to_string(__GNUC_PATCHLEVEL__);
    return name + version;
    #else
    return "Other";
    #endif
}

int main() {
    #if defined _WIN32 || defined _WIN64
        Get_System_Info_Windows();
    #endif
    #if defined __unix || defined __unix__ || defined __linux__ || defined __FreeBSD__
        Get_System_Info_Unix();
        cout << "Total memory: " << getTotalSystemMemoryUnix() << " Mega Bytes" << endl;
    #endif
    cout << "Operating system: " << getOsName() << endl;
    #if defined __unix || defined __unix__ || defined __linux__ || defined __FreeBSD__
        cout << Get_OS_Info("/usr/bin/lsb_release -a");
    #endif
    cout << "C++ version: " << cpp_version() << endl;
    cout << "Compiler name: " << getCompilerName() << endl;
    return 0;
}
