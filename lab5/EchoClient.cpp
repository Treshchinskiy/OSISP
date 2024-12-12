// Echo Client
#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8082
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr{};
    char buffer[BUFFER_SIZE];
    std::string message;

    // Инициализация WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << "\n";
        return 1;
    }

    // Создание клиентского сокета
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(SERVER_PORT);

    // Подключение к серверу
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed with error: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server.\n";

    // Ввод сообщения и обмен данными с сервером
    while (true) {
        std::cout << "Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") break;

        if (send(clientSocket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed with error: " << WSAGetLastError() << "\n";
            break;
        }

        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Завершаем строку
            std::cout << "Server replied: " << buffer << "\n";
        } else if (bytesReceived == 0) {
            std::cout << "Server disconnected.\n";
            break;
        } else {
            std::cerr << "Receive failed with error: " << WSAGetLastError() << "\n";
            break;
        }
    }

    // Закрытие сокета
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}


