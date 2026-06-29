#include "AssetManagement/Compilers/AssetCompilerBase.h"
#include <Windows.h>
#include <fileapi.h>
#include <fstream>
#include <iostream>

void AssetCompilerBase::RunExecutable(std::string& _executableCmd)
{
    // Set process startup info, start process with window hidden
    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;

    // Start process to compile asset
    PROCESS_INFORMATION processInfo;
    bool result = CreateProcessA(NULL, _executableCmd.data(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

    // Check if process started
    if (!result)
    {
        // Process failed to start, print error msg
        DWORD errorCode = GetLastError();
        std::cerr << "Process failed to start! Error code: " << errorCode << std::endl;
    }
    else
    {
        // Process started successfully, wait for it to finish
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
}
