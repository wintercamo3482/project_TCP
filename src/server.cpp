#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>


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


int init_server()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int so_opt = 1;

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &so_opt, sizeof(so_opt));

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.100.124");

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    return serverSocket;
}

int main()
{
    makeDB();

    int serverSocket = init_server();

    listen(serverSocket, 1);

    int epoll_fd = epoll_create1(0); // 입출력 이벤트 저장을 위한 공간
    
    epoll_event ev, events[3];
    ev.events = EPOLLIN | EPOLLET;  // 읽을 데이터가 있는 경우 이벤트를 감지하는데, 엣지 트리거로 설정한다는 뜻
    ev.data.fd = serverSocket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &ev) == -1)    // epoll 안의 파일 디스크럽터를 add/modidfy/delete하는 함수.
    {
        cerr << "서버소켓 추가 실패" << endl;
        return -1;
    }

    while (1)
    {
        int event_cnt = epoll_wait(epoll_fd, events, 10, -1);    // 관심있는 fd에 무슨 일이 발생했는지 조사하여 사건들의 리스트를 배열로 전달.
        cout << event_cnt << endl;

        for (int n = 0; n < event_cnt; ++n)
        {
            if (events[n].data.fd == serverSocket)          // 이벤트 발생한 곳이 서버 소켓. 즉, 클라이언트 연결 or 연결 끊김
            {
                sockaddr_in clientAddr;
                socklen_t addr_size = sizeof(clientAddr);

                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);

                // 클라이언트 소켓을 non-blocking으로 설정. 엣지 트리거를 사용하기 위해 설정
                // setNonBlocking(clientSocket);
                int flag = fcntl(clientSocket, F_GETFL);
                flag |= O_NONBLOCK;

                // 클라이언트 fd와 epoll에 등록
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientSocket;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &ev) == -1)
                    cerr << "클라이언트 " << clientSocket << " 접속 실패" << endl;
                else
                    cout << clientSocket << "번 클라이언트 접속" << endl;
            }
            
            else
            {
                char recv_buffer[1024];

                memset(recv_buffer, 0, sizeof(recv_buffer));

                int bytesReceived = recv(events[n].data.fd, recv_buffer, sizeof(recv_buffer), 0);
             
                if (bytesReceived == 0)
                {
                    close(events[n].data.fd);
                    cout << events[n].data.fd << " 클라이언트 연결 끊김" << endl;
                }
                
                else if (bytesReceived == -1)
                {
                    continue;
                }

                else
                {
                    cout << "클라이언트 " << events[n].data.fd << " : " << recv_buffer << endl;

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
                        send(events[n].data.fd, (char *)&my_database[tmp - 1], sizeof(my_database[tmp - 1]), 0);
                    }
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}