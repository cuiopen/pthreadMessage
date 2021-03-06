/** @file      task.c 
 *  @brief     消息队列，多线程之间的通信
 *  @note      
 *  @note      
 *  @author   zhangyinliang
 *  @date     2017.08.05
 *  @version   v1.0
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <errno.h>

#include "task.h"

#define MSG_FILE "."
#define MAXBUF 255
#define PERM S_IRUSR|S_IWUSR

enum calType{Add=1,Subtract,Multiply,Divide};

//请注意，这里定义的消息数据类型跟原始消息队列数据类型的定义不一致。
struct myMsg
{
    int number1;
    int number2;
    int cal_type;
};

struct msqid_ds msgbuf;      // 获取消息属性的结构体


void *td_receive_fun(void *arg)
{
    //printf("here is receive thread!\n");
    struct myMsg msg;
    int qid, tmpqid, err,i;
    int result=0;
    tmpqid = *((int *)arg);
    qid = tmpqid;
    while (1)
    {
        qid = tmpqid;
        //注意搞清楚，该消息队列接收函数中各个参数的含义
        if (-1 == (err = msgrcv(qid, &msg, sizeof(struct myMsg), 0, 0)) )
        {
            //printf("Receive msg fail!\n");
            continue;
        }
        printf("Receive msg success!\n");
        for(i=0;i<MAX_FUN;i++)
        {
            if(msg.cal_type==out_fun[i].item)
            {
                result=(*out_fun[i].out)(msg.number1,msg.number2);   
                printf("calculation result: %d\n",result);
            }
        }     
    }
    
    pthread_exit((void *)2);
}


void *td_send_fun(void *arg)
{
    //printf("here is send thread!\n");
    struct myMsg msg;
    int qid, qidtmp;
    int err;
    int number1,number2;
    int cal_type;
    qidtmp = *((int *)arg);
    while (1)
    {
        //输入第一个数
        printf("Input the number1: ");
        scanf("%d", &number1);
        printf("number1 = %d\n",number1);
        fflush(stdin);
        fflush(stdout);
        msg.number1=number1;
     
        //选择计算类型
        printf("******please choose the calculation type number******\n");
        printf("1:add\n");
        printf("2:subtract\n");
        printf("3:multiply\n");
        printf("4:divide\n");
        scanf("%d", &cal_type);
        printf("calculation type number= %d\n", cal_type);
        fflush(stdin);
        fflush(stdout);
        msg.cal_type = cal_type;
        
        //输入第二个数
        printf("Input the number2: ");
        scanf("%d", &number2);
        printf("number2 = %d\n",number2);
        fflush(stdin);
        fflush(stdout);
        msg.number2=number2;
        
        //检查数据除法输入是否正确
        while(Divide==(enum calType)cal_type&&0==number2)
        {
            printf("Input wrong, please input the number2 again: ");
            scanf("%d", &number2);
            printf("number2 = %d\n",number2);
            fflush(stdin);
            fflush(stdout);
            msg.number2=number2;
        }
    
        if ( -1 == (msgsnd(qid, &msg, sizeof(struct myMsg), 0)) )
        {
            perror("msg closed! quit the system!\n");
            break;
        }
        printf("Send msg success!\n");
        sleep(2);
    }

    pthread_exit((void *)2);
}

/*消息、发送线程和接收线程的初始化函数*/
void init_task()
{
    pthread_t td_send, td_receive;
    int err1, err2;
    int qid;
    key_t key;
    
    // 创建key
    if ( -1 == (key = ftok(".", 'a')) )   
    {
         perror("Create key error!\n");
         exit (-1);
    }
    
    //创建消息队列
    if ( -1 == (qid = msgget(key, IPC_CREAT | 0777)) )  // | IPC_EXCL 
    {
        perror("message queue already exitsted!\n");
        exit (-1);
    }
    printf("create . open queue qid is %d!\n", qid);
    
    //创建发送消息的线程
    err1 = pthread_create(&td_send, NULL, td_send_fun, &qid);
    if ( 0 != err1 )
    {
        fprintf(stderr, "sed thread create error!\n");
        exit(1);
    }

    //创建接受消息的线程
    err2 = pthread_create(&td_receive, NULL, td_receive_fun, &qid);
    if ( 0 != err2 )
    {
        fprintf(stderr, "receive thread create error!\n");
        exit(1);
    }

    err1 = pthread_join(td_send, NULL);
    if ( 0 != err1 )
    {
        printf("join send thread failed!\n");
        exit(1);
    }

    err2 = pthread_join(td_receive, NULL);
    if ( 0 != err2 )
    {
        printf("join receive thread failed!\n");
        exit(1);
    }
}
