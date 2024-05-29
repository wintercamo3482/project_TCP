#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
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

vector<int> clietns;

int main()
{
    makeDB();                           // 테스트용 DB 생성

    int serverSocket = init_server();   // 서버 초기화

    listen(serverSocket, 1);            // listen()

    fd_set readSet;                     // 소켓을 그룹짓기 위해 사용하는 파일 디스크럽터 선언
    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);
    int fd_max = serverSocket;

    while (1)
    {
        fd_set copySet = readSet; // 현재와 이전의 fd를 비교하기 위한 임시 구조체 생성

        if (select(fd_max + 1, &copySet, nullptr, nullptr, nullptr) == -1)  // fd_set의 소켓 변화 감지.
            cerr << "select 에러" << endl;

        for (int i = 0; i <= fd_max; i++)   // 원본 set에 설정된 소켓 수 만큼 검사
        {
            if (FD_ISSET(i, &copySet))      // 원본 set과 복사한 set을 비교하여 배열의 어느 위치에서 변화가 일어났는지 감지
            {
                if (i == serverSocket)      // 서버소켓에서 변화가 감지. 즉, 새로운 클라이언트 접속 시도를 감지
                {
                    sockaddr_in clientAddr;
                    socklen_t clntAddr_size = sizeof(clientAddr);
                    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clntAddr_size);

                    FD_SET(clientSocket, &readSet);
                    fd_max = max(fd_max, clientSocket);         // 소켓의 수(최신 소켓 번호)를 최신화
                    cout << clientSocket << "가 연결됨"<< endl;
                }
                
                else                        // 클라이언트 소켓에서 변화가 감지
                {
                    char recv_buffer[1024];

                    memset(recv_buffer, 0, sizeof(recv_buffer));

                    int bytesReceived = recv(i, recv_buffer, sizeof(recv_buffer), 0);

                    if (bytesReceived == -1)
                    {
                        continue;
                    }

                    else if (bytesReceived == 0)
                    {
                        cout << "클라이언트 " << i << " 의 연결 끊김" << endl;
                        fd_max -= 1;
                        close(i);
                        FD_CLR(i, &readSet);
                    }

                    else
                    {
                        cout << "클라이언트 " << i << " : " << recv_buffer << endl;

                        istringstream iss(recv_buffer);

                        int tmp;
                        iss >> tmp;

                        if (iss.fail() || tmp < 1 || tmp > 5)   // 클라이언트로부터 입력 받은 값이 유효하지 않을 경우
                        {
                            string send_buffer = "잘못된 명령어. 다시 입력해주세요.";
                            cout << send_buffer.size() << endl;
                            send(i, recv_buffer, bytesReceived, 0);
                        }

                        else
                        {
                            cout << "보낼 데이터 확인... " << my_database[tmp - 1].major << endl;
                            send(i, (char *)&my_database[tmp - 1], sizeof(my_database[tmp - 1]), 0);
                        }

                    }
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}