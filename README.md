### 구성도
![image](https://github.com/wintercamo3482/project_TCP/assets/146147393/32941909-8214-4c7a-8db8-5916fbf7b828)

### 구현 기능
+ 멀티스레드 클라이언트
+ 서버 및 클라이언트 중 한쪽이 종료될 경우, 반대편이 재실행할 때까지 대기
+ 구조체 데이터 송수신

### 필요할 경우
![image](https://github.com/wintercamo3482/project_TCP/assets/146147393/38067aa9-7215-4f6a-98f7-3543a6715d8a)
연결할 서버 PC의 IP와 포트 번호에 맞게 서버 및 클라이언트 코드 수정.

### 빌드 및 실행
```
g++ -o clnt client.cpp
g++ -o serv server.cpp
```
**서버**
```
./server
```
**클라이언트**
```
./clnt
```
