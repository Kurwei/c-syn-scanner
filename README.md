# c-syn-scanner
A low-level TCP SYN port scanner written entirely in C using raw sockets.
This project was developed to gain a deeper understanding of network protocols, system-level programming, and the inner workings of popular scanning tools like Nmap. Instead of relying on the operating system's networking stack, this tool manually crafts IP and TCP headers and analyzes raw network responses.

## Features
* **Raw Sockets:** Bypasses the standard OS networking stack utilizing `SOCK_RAW`.
* **Manual Packet Crafting:** Custom construction of `struct iphdr` and `struct tcphdr`.
* **Kernel Bypass:** Uses the `IP_HDRINCL` socket option to prevent the OS from overwriting custom IP headers.
* **Response Parsing:** Actively listens to the network interface via `recvfrom()` to catch and parse returning `SYN-ACK` (Open) and `RST` (Closed) flags.

## Platform Compatibility
**Linux Only:** This tool is designed for Linux environments (tested on Kali/Ubuntu). It relies on Linux-specific raw socket implementations and header definitions (e.g., bitfields for TCP flags).

## Prerequisites
* GCC Compiler
* Root (`sudo`) privileges are strictly required to open raw sockets.

## Compilation
Compile the source code using `gcc`:
gcc syn_scanner.c -o syn_scanner

## USAGE
The scanner requires exactly four arguments: Source IP, Source Port, Target IP, and Target Port.
sudo ./syn_scanner <source_ip> <source_port> <target_ip> <target_port>

## DISCLAIMER
This tool is created strictly for educational purposes and network protocol analysis. It should only be used on networks and systems where you have explicit permission to conduct scanning activities.
