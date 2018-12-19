#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
/*
 *
*/
int queue[200]; // MAX SIZE = 200
int box = 0; // 생산,  소비할 매개체
int r_size  = 5000; //명령어의 기본크기는 5000
int q_size  = 100;  //큐의 기본크기는 100
int request = 0;
int produce = 0;
int consume = 0;
int produce_num[10]; //생산자쓰레드의 번호를 저장할 배열
int consume_num[10]; //소비자쓰레드의 번호를 저장할 배열
//세마포어변수들
sem_t full;
sem_t empty;
sem_t mutex;
int set(int what, int id){
    int i = 0;
    if(what == 1){ // 생산자 쓰레드
        box++;
        sem_wait(&empty);
        sem_wait(&mutex);
        if(request>=r_size){ //명령어가 모두 실행되면 종료
            sem_post(&mutex);
            sem_post(&full);
            return -1;
        }
        queue[produce%q_size] = 1;
        produce++;
        request++;
        produce_num[id] += 1;
        printf("%6d ", request); //결과출력
        for(i = 0; i<q_size; i++)
            printf("%d",queue[i]);
        printf("\n");
        sem_post(&mutex);
        sem_post(&full);
        return 0;
    }      
    else{ // 소비자 쓰레드
        sem_wait(&full);
        sem_wait(&mutex);//명령어가 모두 실행되면 쓰레드 종료
        if(request>= r_size){
            sem_post(&mutex);
            sem_post(&empty);
            return -1;
        }
        queue[consume%q_size] = 0;
        consume++;
        request++;
        consume_num[id] += 1;
        printf("%6d ", request); // 결과출력
        for(i = 0; i<q_size; i++)
            printf("%d",queue[i]);
        printf("\n");
        sem_post(&mutex);
        sem_post(&empty);
        box--;
        return 0;
    }
}
void *produce_thread(void *tid){ // 생산자 쓰레드
    int id = *((int *)tid);
    int i;
    while(1){
        if(set(1,id)==-1)
            pthread_exit(NULL);
    }
}
 
void *consume_thread(void *tid){
    int id = *((int *)tid); //소비자 쓰레드
    int i;
    while(1){
        if(set(0,id)==-1)
            pthread_exit(NULL);
    }
}
 
int main(int argc, char* argv[]){
    int i;
    int state;
    char* q_str = "-q";
    char* r_str = "-r";
    char* p_str = "-p"; int p_size = 3;
    char* c_str = "-c"; int c_size = 3;
    int status  = 0;
    int p_num[10]; //쓰레드의 번호를 저장할 배열
    int c_num[10];
    pthread_t thread_p[10];
    pthread_t thread_c[10];
    //init setting
    for(i = 0; i<argc; i++){
        if(strcmp(argv[i],q_str)==0)
            q_size = atoi(argv[i+1]);
        if(strcmp(argv[i],r_str)==0)
            r_size = atoi(argv[i+1]);
        if(strcmp(argv[i],p_str)==0)
            p_size = atoi(argv[i+1]);
        if(strcmp(argv[i],c_str)==0)
            c_size = atoi(argv[i+1]);
    }
    if(q_size<1 || q_size>200){
        printf("q_size is not correct\n");
        return -1;
    }
    if(r_size<1 || r_size>50000){
        printf("r_size is not correct\n");
        return -1;
    }
    if(p_size<1 || p_size>10){
        printf("p_size is not correct\n");
        return -1;
    }
    if(c_size<1 || c_size>10){
        printf("c_size is not correct\n");
        return -1;
    }
    for(i = 0; i<p_size; i++)
        p_num[i] = i;
    for(i = 0; i<c_size; i++)
        c_num[i] = i;
    if(sem_init(&full,0,0)!=0){
        printf("sem_init Error\n");
        return 0;
    }
    if(sem_init(&empty,0,q_size)!=0){
        printf("sem_init Error\n");
        return 0;
    }
    if(sem_init(&mutex,0,1)!=0){
        printf("sem_init Error\n");
        return 0;
    }
    //생산자 쓰레드 생성
    for(i = 0; i<p_size; i++){
        status = pthread_create(&thread_p[i], NULL,  produce_thread,(void*) (p_num+i));
        if(status != 0)
            printf("produce : pthread_creat returned error code : %d\n",status);
    }
    //소비자 쓰레드 생성
    for(i = 0; i<c_size; i++){
        status = pthread_create(&thread_c[i], NULL, consume_thread, (void*) (c_num+i));
        if(status != 0)
            printf("consume : pthread_creat returned error code : %d\n",status);
    }
    //쓰레드가 끝날때까지 기다림
     
    for(i = 0; i<p_size; i++){
        pthread_join(thread_p[i], (void**)&status);
    }
     
    for(i = 0; i<c_size; i++){
        pthread_join(thread_c[i], (void**)&status);
    }
    //생산자가 모두 끝나면 결과를 출력
    for(i = 0; i<p_size; i++){
        printf("producer %d:%d\n",i,produce_num[i]);
    }
    printf("\n");
    //소비자가 모두 끝나면 결과를 출력
    for(i = 0; i<c_size; i++){
        printf("consumer %d:%d\n",i,consume_num[i]);
    }
    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&mutex);
     
    return 0;
}


