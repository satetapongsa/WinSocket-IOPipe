# Win32-ReverseShell-Internal-Research 🛡️💻

A proof-of-concept (PoC) project demonstrating Windows API mechanics, Network Socket programming, and Process I/O redirection using C++. 

> **⚠️ DISCLAIMER: Educational and Research Purposes Only**
> This repository is created strictly for academic research, cybersecurity education, and defensive analysis (Blue Teaming). The author does not condone, encourage, or promote any malicious or illegal activities. Always obtain explicit permission before testing any network or system.

## 📌 Project Overview
This project explores how a reverse shell operates at the OS level on Windows. It demonstrates how to utilize the **Winsock API** to establish a TCP connection and how to hijack the standard input/output streams of a Windows process (`cmd.exe`) using the Win32 API. 

This is a fundamental concept in both offensive security (penetration testing) and defensive security (malware analysis and endpoint detection).

## 🧠 Core Logic & Mechanics

The execution flow of the program is divided into three main phases:

### 1. Network Initialization (Winsock)
* Uses `WSAStartup` to initialize the Windows Sockets DLL.
* Creates a TCP socket using `WSASocketA`. 
* Defines the target listener's IP address and port using `sockaddr_in` and connects back to the listening machine via `WSAConnect`.

### 2. I/O Redirection (The Pipe)
* This is the core of the reverse shell. The `STARTUPINFO` structure is manipulated to redirect the standard streams of the command-line process.
* `STARTF_USESTDHANDLES` flag is set.
* `hStdInput`, `hStdOutput`, and `hStdError` are all assigned the handle of the established network socket. This effectively pipes all command inputs and outputs over the network connection.

### 3. Process Creation
* Uses `CreateProcessA` to spawn `cmd.exe`.
* The `SW_HIDE` flag is utilized within `STARTUPINFO` to ensure the process runs invisibly in the background, simulating stealth techniques often observed in real-world scenarios.

## ⚙️ Compilation & Usage (Lab Environment)

### Prerequisites
* A C++ compiler (e.g., MSVC, MinGW).
* A safe, isolated lab environment (Virtual Machines recommended).
* A listener machine (e.g., Kali Linux or any system with `netcat`).

### 1. Setup the Listener
On your analyzing/listener machine, open a terminal and start listening on the specified port (e.g., 4444):
```bash
nc -lvnp 4444
