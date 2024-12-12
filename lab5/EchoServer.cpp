
// Echo Server
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8082
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr{}, clientAddr{};
    char buffer[BUFFER_SIZE];
    int clientAddrLen = sizeof(clientAddr);

    // Инициализация WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << "\n";
        return 1;
    }

    // Создание серверного сокета
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Установка параметров для повторного использования порта
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Failed to set socket options. Error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Привязка сокета
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Прослушивание входящих соединений
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port " << SERVER_PORT << "...\n";

    // Ожидание подключения клиента
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected.\n";

    // Эхо-сервис: прием данных и их отправка обратно
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Receive failed with error: " << WSAGetLastError() << "\n";
            break;
        }
        if (bytesReceived == 0) {
            std::cout << "Client disconnected.\n";
            break;
        }

        buffer[bytesReceived] = '\0'; // Завершаем строку
        std::cout << "Received: " << buffer << "\n";

        if (send(clientSocket, buffer, bytesReceived, 0) == SOCKET_ERROR) {
            std::cerr << "Send failed with error: " << WSAGetLastError() << "\n";
            break;
        }
    }

    // Закрытие сокетов
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
