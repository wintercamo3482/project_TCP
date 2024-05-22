#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
using namespace std;

vector<int> clients;

void messagesHandle(int clientSock)
{
    char recv_buffer[1024];
    string send_buffer;

    while (1)
    {
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int bytesReceived = recv(clientSock, recv_buffer, sizeof(recv_buffer), 0);
        
        if (bytesReceived <= 0)
        {
            cerr  << clientSock << "의 연결 끊김." << endl;
            break;
        }

        cout << clientSock << " : " << recv_buffer << endl;

        string send_buffer(recv_buffer, bytesReceived);        
        
        send(clientSock, send_buffer.c_str(), send_buffer.size(), 0);
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

    cout << "Client " << clientSock << " 가 나감." << endl;
}

int main()
{
    // socket ~ bind
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int so_opt = 1;

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &so_opt, sizeof(so_opt));    // 서버를 강제 종료 -> 재실행을 할 경우, 포트를 재사용하기 위해 필요.

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.100.124");

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // listen
    listen(serverSocket, 1);

    while (1)   // 클라이언트 accept
    {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        // 무언가가 접속을 시도하기 전까진 accept에서 대기 상태.
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        
        if (clientSocket == -1)
        {
            cerr << "클라이언트 접속 실패" << endl;
            continue;
        }
        
        clients.push_back(clientSocket);
        cout << "연결된 클라이언트 번호 / 총 클라이언트 : " << clientSocket << " / " << clients.size() << endl;

        // 연결 성공한 클라이언트에 대해선 send/recv 용도의 스레드를 생성해서 부여함.
        thread(messagesHandle, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}