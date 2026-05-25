#include <windows.h>
#include <winerror.h>
#include <psapi.h>
#include <stdio.h>

#define STOP_ARG "xakep"
#define LOCAL_BLOCKDLLPOLICY

BOOL CreateProcessWithBlockDllPolicy(LPSTR lpProcessPath, DWORD* dwProcessId, HANDLE* hProcess, HANDLE* hThread)
{
    STARTUPINFOEXA SiEx = { 0 };
    PROCESS_INFORMATION Pi = { 0 };
    SIZE_T sAttrSize = 0;
    
    if (lpProcessPath == NULL)
        return FALSE;
    
    ZeroMemory(&SiEx, sizeof(STARTUPINFOEXA));
    ZeroMemory(&Pi, sizeof(PROCESS_INFORMATION));
    
    SiEx.StartupInfo.cb = sizeof(STARTUPINFOEXA);
    SiEx.StartupInfo.dwFlags = EXTENDED_STARTUPINFO_PRESENT;
    
    InitializeProcThreadAttributeList(NULL, 1, 0, &sAttrSize);
    
    LPPROC_THREAD_ATTRIBUTE_LIST pAttrBuf = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sAttrSize);
    
    if (!pAttrBuf)
        return FALSE;
    
    if (!InitializeProcThreadAttributeList(pAttrBuf, 1, 0, &sAttrSize))
    {
        printf("[!] InitializeProcThreadAttributeList Failed: %d\n", GetLastError());
        HeapFree(GetProcessHeap(), 0, pAttrBuf);
        return FALSE;
    }
    
    DWORD64 dwPolicy = PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;
    
    if (!UpdateProcThreadAttribute(pAttrBuf, 0, PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY, &dwPolicy, sizeof(DWORD64), NULL, NULL))
    {
        printf("[!] UpdateProcThreadAttribute Failed: %d\n", GetLastError());
        DeleteProcThreadAttributeList(pAttrBuf);
        HeapFree(GetProcessHeap(), 0, pAttrBuf);
        return FALSE;
    }
    
    SiEx.lpAttributeList = pAttrBuf;
    
    if (!CreateProcessA(NULL, lpProcessPath, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &SiEx.StartupInfo, &Pi))
    {
        printf("[!] CreateProcessA Failed: %d\n", GetLastError());
        DeleteProcThreadAttributeList(pAttrBuf);
        HeapFree(GetProcessHeap(), 0, pAttrBuf);
        return FALSE;
    }
    
    *dwProcessId = Pi.dwProcessId;
    *hProcess = Pi.hProcess;
    *hThread = Pi.hThread;
    
    DeleteProcThreadAttributeList(pAttrBuf);
    HeapFree(GetProcessHeap(), 0, pAttrBuf);
    
    return TRUE;
}

int main(int argc, char* argv[])
{
    DWORD dwProcessId = 0;
    HANDLE hProcess = NULL, hThread = NULL;
    
    #ifdef LOCAL_BLOCKDLLPOLICY
        if (argc == 2 && (strcmp(argv[1], STOP_ARG) == 0))
        {
            printf("[+] Process Is Now Protected With The Block Dll Policy\n");
            printf("[*] PID: %d\n", GetCurrentProcessId());
            printf("[*] Only Microsoft-signed DLLs can be loaded\n");
            printf("[*] Injection attempts will FAIL\n\n");
            
            int i = 0;
            while (true)
            {
                printf("Processing - %d\n", i++);
                Sleep(1000);
            }
        }
        else
        {
            printf("[!] Local Process Is Not Protected With The Block Dll Policy\n");
            
            CHAR pcFilename[MAX_PATH * 2];
            GetModuleFileNameA(NULL, (LPSTR)&pcFilename, MAX_PATH * 2);
            
            CHAR pcBuffer[MAX_PATH * 2];
            sprintf_s(pcBuffer, sizeof(pcBuffer), "\"%s\" %s", pcFilename, STOP_ARG);
            
            CreateProcessWithBlockDllPolicy(pcBuffer, &dwProcessId, &hProcess, &hThread);
            
            printf("[i] Process Created With Pid %d\n", dwProcessId);
            printf("[i] Protected process is running. Exiting...\n");
        }
    #endif
    
    return 0;
}