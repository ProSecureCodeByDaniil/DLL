#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <tlhelp32.h>

// Функция проверки загруженных DLL
BOOL IsMaliciousDllLoaded()
{
    HMODULE hMods[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            char szModName[MAX_PATH];
            if (GetModuleFileNameExA(GetCurrentProcess(), hMods[i], szModName, sizeof(szModName)))
            {
                // Список запрещенных DLL
                if (strstr(szModName, "VirusDLL.dll") != NULL ||
                    strstr(szModName, "DllInjector") != NULL)
                {
                    printf("[!!!] MALICIOUS DLL DETECTED: %s\n", szModName);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// Функция для мониторинга новых загруженных модулей через Toolhelp32Snapshot
DWORD WINAPI MonitoringThread(LPVOID lpParam)
{
    HANDLE hProcess = GetCurrentProcess();
    DWORD dwProcessId = GetCurrentProcessId();
    
    // Список уже загруженных модулей
    HMODULE hMods[1024];
    DWORD cbNeeded;
    char szModName[MAX_PATH];
    
    while (true)
    {
        Sleep(100); // Проверяем каждые 100 мс
        
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
        {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName)))
                {
                    if (strstr(szModName, "VirusDLL.dll") != NULL)
                    {
                        printf("\n[!!!] CRITICAL: VirusDLL.dll detected!\n");
                        printf("[!!!] Terminating process to prevent damage...\n");
                        
                        // Показываем предупреждение
                        MessageBoxA(NULL, "Malicious DLL detected! Process will be terminated for security.", 
                                    "SECURITY ALERT", MB_OK | MB_ICONERROR);
                        
                        // Немедленное завершение процесса
                        ExitProcess(1);
                    }
                }
            }
        }
    }
    
    return 0;
}

int main()
{
    printf("=== Target Process with Reactive Protection ===\n");
    printf("[*] PID: %d\n", GetCurrentProcessId());
    printf("[*] Monitoring for malicious DLLs...\n");
    printf("[*] Process will terminate if VirusDLL.dll is loaded\n\n");
    
    // Запускаем поток мониторинга
    HANDLE hMonitor = CreateThread(NULL, 0, MonitoringThread, NULL, 0, NULL);
    
    int i = 0;
    while (true)
    {
        printf("Processing - %d\n", i++);
        Sleep(1000);
    }
    
    return 0;
}