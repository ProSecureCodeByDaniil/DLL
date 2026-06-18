#include <windows.h>
#include <psapi.h>
#include <stdio.h>

// Функция проверки загруженных DLL (максимально быстрая)
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
                if (strstr(szModName, "VirusDLL.dll") != NULL)
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// Мониторинг с высокой частотой
DWORD WINAPI MonitoringThread(LPVOID lpParam)
{
    HANDLE hProcess = GetCurrentProcess();
    
    while (true)
    {
        // Проверяем КАЖДЫЕ 10 МИЛЛИСЕКУНД (вместо 100)
        Sleep(10);
        
        if (IsMaliciousDllLoaded())
        {
            // НЕМЕДЛЕННОЕ завершение без MessageBox
            // Используем TerminateProcess для мгновенной остановки
            printf("\n[!!!] VirusDLL.dll detected! Terminating NOW!\n");
            
            // Отключаем буферизацию вывода
            fflush(stdout);
            
            // Мгновенное завершение процесса
            TerminateProcess(hProcess, 1);
            
            // Этот код не выполнится
            return 0;
        }
    }
    
    return 0;
}

int main()
{
    // Отключаем буферизацию вывода для скорости
    setvbuf(stdout, NULL, _IONBF, 0);
    
    printf("=== Target Process with FAST Reactive Protection ===\n");
    printf("[*] PID: %d\n", GetCurrentProcessId());
    printf("[*] Ultra-fast monitoring active (10ms intervals)\n");
    printf("[*] Process will be TERMINATED IMMEDIATELY if VirusDLL.dll is loaded\n\n");
    
    // Устанавливаем высокий приоритет потоку мониторинга
    HANDLE hMonitor = CreateThread(NULL, 0, MonitoringThread, NULL, 0, NULL);
    SetThreadPriority(hMonitor, THREAD_PRIORITY_HIGHEST);
    
    int i = 0;
    while (true)
    {
        printf("Processing - %d\n", i++);
        
        // Дополнительная проверка в основном потоке
        if (IsMaliciousDllLoaded())
        {
            printf("[!!!] Direct detection! Terminating...\n");
            TerminateProcess(GetCurrentProcess(), 1);
        }
        
        Sleep(1000);
    }
    
    return 0;
}