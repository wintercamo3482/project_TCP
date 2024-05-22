#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <unistd.h>
using namespace std;

bool is_connected = false;

struct connectionInfo
{
    int sock;
    sockaddr_in addr;
};

connectionInfo info;

void sockect_init()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.100.124");

    info = {sock, serverAddr};

    // connectionInfo tmp = {sock, serverAddr};
    // return tmp;
}

void connectToServer(connectionInfo info)
{
    while(1)
    {
        if (connect(info.sock, (sockaddr*)&info.addr, sizeof(info.addr)) == -1)
        {
            cerr << "Waiting..." << endl;
            is_connected = false;
            sleep(2);
        }

        else
        {
            cerr << "Done!!!" << endl;
            is_connected = true;
            break;
        }
    }
}

// void handleMessages(int sock)
// {
//     string message;
//     char buffer[1024];
    
//     while (getline(cin, message))
//     {
//         send(sock, message.c_str(), message.size(), 0);

//         memset(buffer,0, sizeof(buffer));

//         int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

//         if (bytesReceived <= 0)
//         {
//             cerr << "연결 끊김 및 에러 발생" << endl;
//         }
//         else
//         {
//             cout << sock << "가 보낸 메시지 : " << endl;
//         }
//     }
// }

int main()
{
    string message;
    char buffer[1024];

    while(1)
    {
        cout << "시작 or 외부 while 재시작" << endl;
        sockect_init();
        connectToServer(info);

        while (getline(cin, message))
        {
            send(info.sock, message.c_str(), message.size(), 0);
            memset(buffer,0, sizeof(buffer));

            int bytesReceived = recv(info.sock, buffer, sizeof(buffer), 0);
            
            if (bytesReceived <= 0)
            {
                cerr << "연결 끊김 및 에러 발생" << endl;
                break;
            }
            else
            {
                cout << info.sock << "가 보낸 메시지 : " << endl;
            }
        }
        cout << "내부 while 탈출" << endl;
        close(info.sock);
    }
 
    // thread receiver(receiveMessages, sock);
    // thread sender(sendMessages, sock);
    // thread handler(handleMessages, info.sock);
    // receiver.join();
    // sender.detach();

    // close(info.sock);
    return 0;
}