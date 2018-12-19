#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 100     // 메시지전송 최대 길이
#define NAMESIZE 50     // 사용자명   최대 길이

void * send_message(void *arg);         // 메시지 전송 thread 함수
void * recv_message(void *arg);         // 메시지 수신 thread 함수
void   error_handling(char *message);   // 에러 처리 함수

char name[NAMESIZE] = "";   // 사용자명 변수 선언
char message[BUFSIZE];      // 송/수신 할 메시지 변수 선언

int main(int argc, char *argv[])
{
    int sock;                           // 접속할 서버 서켓 핸들
    struct sockaddr_in serv_addr;       // 접속할 서버 서켓의 주소 구조체
    pthread_t snd_thread, rcv_thread;   // 전송 thread ID, 수신할 thread ID 생성(thread_t 자료형 선언)
    void *thread_result;                // thread 함수에서 반환되는 반환값 변수 선언

    // 실행 시 입력 인자값 체크
    if(argc!=4){
        printf("Usage : %s <IP> <PORT> <name>\n", argv[0]);
        exit(1);
    }

    // 사용자명을 복사
    strcpy(name, argv[3]);

    // 서버소켓 핸들 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1)
        error_handling("socket() error");

    // 서버소켓 주소 구조체 초기화 및 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons((unsigned short)atoi(argv[2]));

    // 서버 주소구조체 정보를 사용하여 서버에 연결요청
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("connet() error");

    // 메시지 전송 / 메시지 수신 thread 함수 호출과 함께 thread 생성
    // 인자값으로 서버소켓 핸들을 전달하기 때문에, 함수 내에서 send / recv할 수 있다
    pthread_create(&snd_thread, NULL, send_message, (void *)sock);
    pthread_create(&rcv_thread, NULL, recv_message, (void *)sock);

    // 메시지 전송 / 메시지 수신 thread가 끝났때 까지 대기함
    pthread_join(snd_thread, &thread_result);
    pthread_join(rcv_thread, &thread_result);

    //서버 소켓 종료함
    close(sock);

    return EXIT_SUCCESS;
}

// 메시지 전송 thread 함수
void * send_message(void * arg)
{
    // 서버소켓 핸들 값을 int형으로 형변환하여 변수로 선언.
    int sock = (int)arg;
    // 사용자명과 메시지를 저장할 전송메시지내용을 변수로 선언
    char name_message[NAMESIZE+BUFSIZE];

    // 전송할 메시지를 입력을 반복시킨다.
    while(1){
        // 전송할 메시지 입력을 받아들인다.
        fgets(message, BUFSIZE, stdin);
        // 입력값이 "q\n"일 경우 thread함수를 빠져나와 실질적으로 main함수로 돌아간다.
        if(!strcmp(message, "q\n")){
            close(sock);
             exit(0);
        }
        // 입력한 메시지와 사용자 이름을 조합하여 전송할 메시지를 완성한다.
        strcpy(name_message, "[");
        strcat(name_message, name);
        strcat(name_message, "] ");
        strcat(name_message, message);
        // 조합된 메시지를 전송한다.
        write(sock, name_message, strlen(name_message));
    }
}

// 메시지 수신 thread 함수
void * recv_message(void * arg)
{
    // 서버소켓 핸들 값을 int형으로 형변환하여 변수로 선언
    int sock = (int)arg;
    // 사용자명과 메시지를 저장할 수신메시지내용을 변수로 선언
    char name_message[NAMESIZE+BUFSIZE];
    int str_len;    // 수신 메시지의 길이를 저장할 변수 선언
    int rtn;        // 연결 종료 후 thread함수를 빠져 나갈 시 반환되는 값을 변수로 선언

    // 메시지 읽기 시도를 계속 반복한다.
    while(1){
        // 메시지를 읽어 들인다.
        str_len = read(sock, name_message, NAMESIZE+BUFSIZE-1);
        // 메시지의 길이가 -1(연결종료)이면 thread함수를 빠져 나간다.
        if(str_len==-1)
        {
            rtn = 1;    // thread함수의 반환 값을 1로 설정한다.
            // thread함수의 반환형이 void* 형이기 때문에 (void*)형으로 현변환을 한다.
            return (void *)rtn;
        }
        name_message[str_len] = 0;  // 수신된 메시지의 끝에 NULL종료문자를 지정한다.
        fputs(name_message, stdout);    // 수신된 메시지를 표준출력(모니터)에 출력한다.
    }
}

// 에러 발생시 처리할 메시지 처리 함수
void error_handling(char * message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

