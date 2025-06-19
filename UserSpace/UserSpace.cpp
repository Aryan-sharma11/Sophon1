// UserSpace.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#include <windows.h>
#include <iostream>
#include <tchar.h>

bool InstallAndStartDriver(const wchar_t* serviceName, const wchar_t* driverPath)
{
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) {
        std::wcerr << L"OpenSCManager failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    SC_HANDLE hService = CreateService(
        hSCManager,
        serviceName,
        serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        driverPath,
        nullptr, nullptr, nullptr, nullptr, nullptr);

    if (!hService) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            std::wcout << L"Service already exists. Opening it...\n";
            hService = OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);
            if (!hService) {
                std::wcerr << L"OpenService failed. Error: " << GetLastError() << std::endl;
                CloseServiceHandle(hSCManager);
                return false;
            }
        }
        else {
            std::wcerr << L"CreateService failed. Error: " << err << std::endl;
            CloseServiceHandle(hSCManager);
            return false;
        }
    }

    // Start the driver
    if (!StartService(hService, 0, nullptr)) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_ALREADY_RUNNING) {
            std::wcout << L"Service already running.\n";
        }
        else {
            std::wcerr << L"StartService failed. Error: " << err << std::endl;
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            return false;
        }
    }
    else {
        std::wcout << L"Driver started successfully.\n";
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return true;
}
#include <windows.h>
#include <iostream>

bool RunCommand(const wchar_t* command)
{
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    wchar_t cmdline[512];
    wcscpy_s(cmdline, command); // cmdline must be modifiable

    if (!CreateProcessW(
        nullptr,          // No module name
        cmdline,          // Command line
        nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE,
        nullptr, nullptr,
        &si, &pi))
    {
        std::wcerr << L"CreateProcess failed: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

int wmain()
{
    const wchar_t* driverName = L"ProcessBlocker";

    const wchar_t* driverPath = L"C:\\DriverTest\\Drivers\\ProcessBlocker.sys";

    if (InstallAndStartDriver(driverName, driverPath)) {
        std::wcout << L" Process Driver installation succeeded.\n";
    }
    else {
        std::wcerr << L" Process Driver installation failed.\n";
    }
 
    if (RunCommand(L"pnputil.exe /add-driver C:\\DriverTest\\FileBlocker.inf /install")) {
        
        std::wcerr << L" Starting file blocker\n";

        if (RunCommand(L"sc start FileBlocker")) {
            std::wcerr << L" Started file blocker\n";
        }
        else {
            std::wcerr << L" Starting file blocker failed\n";
        }
    }
    else {
        std::wcerr << L" File Blocker Driver installation failed.\n";
    }
        
    system("pause");
    return 0;
}
