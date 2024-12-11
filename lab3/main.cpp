#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

std::wstring getExecutablePath(const std::wstring& name) {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::wstring exePath(buffer);
    size_t pos = exePath.find_last_of(L"\\/");
    return exePath.substr(0, pos + 1) + name + L".exe";
}


bool createProcess(const std::wstring& exeName) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    BOOL isSuccess = CreateProcessW(
        exeName.c_str(),  
        NULL,             
        NULL,             
        NULL,            
        FALSE,            
        0,                
        NULL,             
        NULL,             
        &si,              
        &pi               
    );

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::vector<std::wstring> processNames = { L"DataGenerator", L"Sorter", L"OutputProcess" };

    for (const auto& name : processNames) {
        std::wstring exePath = getExecutablePath(name);
        if (!createProcess(exePath)) {
            std::wcerr << L"Failed to start process: " << name << std::endl;
            return 1;
        }
        else {
            std::wcout << L"Process started: " << name << std::endl;
        }
        Sleep(500);
    }

    return 0;
}
