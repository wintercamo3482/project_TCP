#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <unistd.h>
using namespace std;

#pragma pack(push, 1)
struct my_data
{
    short id;
    char major[64];
    float age;
};
#pragma pack(pop)

struct connectionInfo
{
    int sock;
    sockaddr_in addr;
};

connectionInfo info;

void connectToServer()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.100.29");

    info.sock = sock;
    info.addr = serverAddr;

    while(1)
    {
        int connect_sock = connect(info.sock, (sockaddr*)&info.addr, sizeof(info.addr));
        if (connect_sock == -1)
        {
            cerr << " 대기 중..." << endl;
            sleep(2);
        }

        else
        {
            cerr << "연결 완료" << endl;
            break;
        }
    }
}

void receiveMessages(int sock)
{
    my_data recv_buffer;

    while (true)
    {
        memset(&recv_buffer, 0, sizeof(recv_buffer));
        
        int bytesReceived = recv(info.sock, &recv_buffer, sizeof(recv_buffer), 0);
        
        if (bytesReceived <= 0)
        {
            cerr << "접속 중..." << endl;
            close(info.sock);
            connectToServer();
        }
        else if (bytesReceived == sizeof(my_data))
        {
            cout << bytesReceived << " : " << sizeof(bytesReceived) << endl;
            cout << recv_buffer.id << '\t' << recv_buffer.major << '\t' << recv_buffer.age << endl;
        }
        else
        {
            cerr << "유효하지 않는 입력. 1 ~ 5까지 숫자 중 하나만 입력." << endl;
        }

    }

}

void sendMessages(int sock)
{
    string message;

    while(getline(cin, message))
        send(info.sock, message.c_str(), message.size(), 0);
    
}

int main()
{
    string message;
    char buffer[1024];

    connectToServer();
        thread receiver(receiveMessages, info.sock);
        thread sender(sendMessages, info.sock);

        receiver.join();
        sender.detach();
    
    return 0;
}