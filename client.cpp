#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define SEND_FLAGS 0
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define closesocket close
#define SEND_FLAGS MSG_NOSIGNAL
#endif

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

namespace ClientApp { // <--- ПРЯЧЕМ ВСЁ В ПРОСТРАНСТВО ИМЕН

    const int PORT = 9000;
    const char* SERVER_IP = "127.0.0.1";

    bool sendLine(SOCKET s, const string& text)
    {
        string data = text + "\n";
        size_t sent = 0;
        while (sent < data.size()) {
            int n = send(s, data.c_str() + sent, (int)(data.size() - sent), SEND_FLAGS);
            if (n <= 0) return false;
            sent += n;
        }
        return true;
    }

    bool recvLine(SOCKET s, string& line)
    {
        line.clear();
        char c;
        while (true) {
            int n = recv(s, &c, 1, 0);
            if (n <= 0) return false;
            if (c == '\n') return true;
            if (c != '\r') line += c;
        }
    }

    void runClient()
    {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cout << "WSAStartup failed" << endl;
            return;
        }
        SetConsoleOutputCP(CP_UTF8);
#endif

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            cout << "socket() error" << endl;
            return;
        }

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        cout << "Connecting to " << SERVER_IP << ":" << PORT << "..." << endl;
        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cout << "connect() failed. Is the server running?" << endl;
            closesocket(sock);
            return;
        }

        string line;
        if (recvLine(sock, line)) {
            cout << "Server: " << line << endl;
        }

        string command;
        while (true) {
            cout << "> ";
            if (!getline(cin, command)) break;
            if (command.empty()) continue;

            if (!sendLine(sock, command)) {
                cout << "send() failed" << endl;
                break;
            }

            if (!recvLine(sock, line)) {
                cout << "Server disconnected" << endl;
                break;
            }
            cout << line << endl;

            if (command == "QUIT" || command == "quit") break;
        }

        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
    }

} // конец namespace ClientApp

int main() {
    ClientApp::runClient();
    return 0;
}