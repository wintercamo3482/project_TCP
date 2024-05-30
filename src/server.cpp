#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sstream>
#include <unistd.h>

using namespace std;

#pragma pack(push, 1)
struct my_data
{
    short id;
    char major[64];
    float grade;

    my_data(short id, const char* major, float grade)
    {
        strncpy(this->major, major, sizeof(this->major) - 1);

        this->id = id;
        this->major[sizeof(this->major) - 1] = '\0';
        this->grade = grade;        
    }
};
#pragma pack(pop)

vector<my_data> my_database;

void makeDB()
{
    my_database.push_back({1, "전자공학과", 4.5f});
    my_database.push_back({2, "기계공학과", 3.5f});
    my_database.push_back({3, "생명공학과", 2.7f});
    my_database.push_back({4, "항공우주공학과", 1.9f});
    my_database.push_back({5, "전기공학과", 4.0f});
}

int main(int argc, char** argv)
{
    makeDB();

    // 서버 socket ~ listen
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int so_opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &so_opt, sizeof(so_opt));

    sockaddr_in serverAddr;

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    listen(serverSocket, 1);

    // epoll_create
    int epoll_fd = epoll_create1(0);    // 파일 디스크립터를 관리하는 자료구조 생성.

    epoll_event ev, event;
    ev.events = EPOLLIN;
    ev.data.fd = serverSocket;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &ev);    // epoll 안의 서버 소켓 FD를 add.

    while (1)
    {
        if (epoll_wait(epoll_fd, &event, 2, -1) == -1)
            cerr << "epoll_wait 에러" << endl;

        if (event.data.fd == serverSocket)
        {
            sockaddr_in clientAddr;
            socklen_t addr_size = sizeof(clientAddr);

            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);

            ev.events = EPOLLIN;
            ev.data.fd = clientSocket;

            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &ev) != -1)
                cout << clientSocket << "번 클라이언트 접속" << endl;

        }

        else
        {
            char recv_buffer[1024];

            memset(recv_buffer, 0, sizeof(recv_buffer));

            int bytesReceived = recv(event.data.fd, recv_buffer, sizeof(recv_buffer), 0);

            if (bytesReceived == 0)
            {
                close(event.data.fd);
                cout << event.data.fd << " 클라이언트 연결 끊김" << endl;
            }

            else if (bytesReceived == -1)
            {
                continue;
            }
            
            else
            {
                cout << "클라이언트 " << event.data.fd << " : " << recv_buffer << endl;
                
                istringstream iss(recv_buffer);

                int tmp;
                iss >> tmp;

                if (iss.fail() || tmp < 1 || tmp > 5)   // 클라이언트로부터 입력 받은 값이 유효하지 않을 경우
                {
                    string send_buffer = "잘못된 명령어. 다시 입력해주세요.";
                    cout << send_buffer.size() << endl;
                }
                
                else
                {
                    cout << "보낼 데이터 확인... " << my_database[tmp - 1].major << endl;
                    send(event.data.fd, (char *)&my_database[tmp - 1], sizeof(my_database[tmp - 1]), 0);
                }
            }
        }
    }

    close(serverSocket);
    close(epoll_fd);
    return 0;
}