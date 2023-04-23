# DLLInjection
DLL Injection refactored.

# DLL Injection

This program demonstrates how to inject a DLL into a process on Windows.

## Prerequisites

- Windows operating system
- Visual Studio or another C++ compiler
- A target process to inject the DLL into

## Usage

1. Compile the program using a C++ compiler, such as Visual Studio.
2. Open the Command Prompt and navigate to the directory containing the compiled program.
3. Run the program with the following command, where `<pid>` is the process ID of the target process:

   ```
   program.exe <pid>
   ```

   For example:

   ```
   program.exe 1234
   ```

   This will inject the `evilm64.dll` file, located in `C:\experiments`, into the process with the specified PID.

## How it works

The program uses the following steps to inject the DLL into the remote process:

1. Open a handle to the remote process using the `OpenProcess` function.
2. Allocate memory in the remote process using the `VirtualAllocEx` function.
3. Write the path of the DLL to the allocated remote memory using the `WriteProcessMemory` function.
4. Retrieve the address of the `LoadLibraryW` function in the remote process using the `GetProcAddress` function.
5. Create a remote thread in the target process using the `CreateRemoteThread` function, passing the address of the `LoadLibraryW` function and the address of the remote buffer containing the DLL path as arguments.
6. Wait for the thread to finish.
7. Free the allocated remote memory and close the process handle.

The program includes error checking at each step to handle potential failures.


```c++
    if (!threadStartRoutineAddress) {
        std::cerr << "Failed to get thread start routine address: " << GetLastError() << std::endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }
```

The program retrieves the address of the `LoadLibraryW` function in the remote process by calling `GetProcAddress` with the `Kernel32` module handle. If the function returns `NULL`, the program prints an error message to `std::cerr` indicating the error code returned by `GetLastError`, frees the allocated remote memory using `VirtualFreeEx`, closes the process handle, and returns a non-zero value to indicate failure.

```c++
    HANDLE remoteThread = CreateRemoteThread(processHandle, nullptr, 0, threadStartRoutineAddress, remoteBuffer, 0, nullptr);
    if (!remoteThread) {
        std::cerr << "Failed to create remote thread: " << GetLastError() << std::endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }
```

The program creates a remote thread in the target process using `CreateRemoteThread`, passing the address of the `LoadLibraryW` function and the address of the remote buffer containing the DLL path as arguments. If the function returns `NULL`, the program prints an error message to `std::cerr` indicating the error code returned by `GetLastError`, frees the allocated remote memory using `VirtualFreeEx`, closes the process handle, and returns a non-zero value to indicate failure.

```c++
    CloseHandle(remoteThread);
    VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
    CloseHandle(processHandle);
    return 0;
}
```

If the thread was successfully created, the program closes the handle to the remote thread, frees the allocated remote memory using `VirtualFreeEx`, closes the process handle, and returns a zero value to indicate success.
