#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
using namespace std;

vector<int> clients;

void handleClient(int clientSock)
{
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSock, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0)
        {
            cerr  << clientSock << "의 연결 끊김." << endl;
            break;
        }

        cout << buffer << endl;
        
        string message(buffer, bytesReceived);
        send(clientSock, message.c_str(), message.size(), 0);
    }

    int i = 0;
    for (auto &clnt : clients)
    {
        if (clnt == clientSock)
        {
            clients.erase(clients.begin() + i);
            break;
        }
        ++i;
    }
    close(clientSock);
    std::cout << "Client " << clientSock << " 가 나감." << std::endl;
}

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.100.124");

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        
        if (clientSocket == -1)
        {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        clients.push_back(clientSocket);
        cout << "Client connected: " << clientSocket << endl;
        thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}