#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr *sockaddrp;
//存儲客戶端地址的結構體數組
struct sockaddr_in src_addr[50];
socklen_t src_len = sizeof(src_addr[0]);
//連接後記錄confd數組
int confd[50] = {};
//設置連接人數
struct node
{
    char name[10];
    int state; //1=quit  0=join
    char no[10];
};
int count = 0;
//char nametable[1000];
int secret = 0;
struct node table[50];

void *broadcast(void *indexp)
{
    int index = *(int *)indexp;
    char buf_rcv[255] ;
    char buf_rcv2[255] ;
    char buf_snd[255] ;
    char buf_snd1[255] ;
    char buf_snd2[255] ;
    char buf_snd3[255] ;
    char buf_snd4[255] ;
    char name2[30] ;
    FILE *fp;
    //第一次讀取用戶姓名
    char name[20]={};
    int ret = recv(confd[index], name, sizeof(name), 0);
    strcpy(name2, name);
    name2[strlen(name2)] = '\n';
    strcpy(table[count-1].name, name2);//
    table[count-1].state = 0;
    if (ret < 0 )
    {
        perror("recv");
        close(confd[index]);
        return 0;
    }
    sprintf(buf_snd, "User %s(ID:%d)  join the chat", name, index);
    sprintf(table[count-1].no,"%d",index);
	for (int i = 0; i < count; i++)
            send(confd[i], buf_snd, strlen(buf_snd), 0);
    while (1)
    {
        memset(buf_rcv,0,sizeof(char));
        memset(buf_rcv2,0,sizeof(char));
        memset(buf_snd,0,sizeof(char));
        memset(buf_snd2,0,sizeof(char));
        memset(buf_snd3,0,sizeof(char));
        memset(buf_snd4,0,sizeof(char));
        bzero(buf_rcv, sizeof(buf_rcv));
        recv(confd[index], buf_rcv, sizeof(buf_rcv), 0);

        //判斷是否退出
        if (strcmp("quit", buf_rcv)==0)
        {
            sprintf(buf_snd1, "%s leave this chat room", name);
            for (int i = 0; i < count; i++)
            {
                if (i == index || confd[i]==0) continue;
                send(confd[i], buf_snd1, strlen(buf_snd1), 0);
            }
            table[index + 1].state = 1;
            confd[index] = -1;
            pthread_exit(0);
        }
        else if (strcmp("list", buf_rcv)==0)
        {
            strcat(buf_snd2, "\n---list---\n");
            for (int i = 0; i <count; i++)
            {
                if (table[i].state != 1){
		    strcat(buf_snd2,"user");
		    strcat(buf_snd2,table[i].no);
		    strcat(buf_snd2," : ");
            strcat(buf_snd2, table[i].name);
                }
            }
            strcat(buf_snd2, "---list---\n");
            printf("%s",buf_snd2);
            for (int i = 0; i < count; i++)
            {
                if (i == index || confd[i]==0)
                {
                    send(confd[i], buf_snd2, strlen(buf_snd2), 0);
                    continue;
                }
            }
            continue;
        }
        if (strstr(buf_rcv, "secret") != NULL || strstr(buf_rcv, "send") || strstr(buf_rcv, "NO") || strcmp(buf_rcv, "YES") == 0)
        {
            char *delim = ",";
            char *num;
            char *talk;
            char *unum;
            int user;
            if (strstr(buf_rcv, "secret") != NULL || strstr(buf_rcv, "send"))
            {
                num = strtok(buf_rcv, delim);
                unum = strtok(NULL, delim);
                talk = strtok(NULL, delim);
                user = atoi(unum);
                sprintf(buf_snd3, "(secret)%s:%s", name, talk);
            }
            if (strstr(buf_rcv, "secret") != NULL)
            {
                for (int i = 0; i < count; i++)
                {
                    if (i == index || 0 == confd[i]) continue;
                    if (i == user)
                    {
                        send(confd[i], buf_snd3, strlen(buf_snd3), 0);
                        break;
                    }
                }
            }
            else if (strstr(buf_rcv, "send") || strstr(buf_rcv, "NO") || strcmp(buf_rcv, "YES") == 0)
            {
                for (int i = 0; i <= count; i++)
                {
                    //printf("index = %d\n conf = %d\n", index, *confd);
                    if (i == index || 0 == confd[i]) continue;
                    if (i == user)
                    {
                        if (strcmp(buf_rcv, "NO") == 0 || strcmp(buf_rcv, "YES") == 0)
                        {
                            //printf("TEST\n");
                            continue;
                        }
                        fp = fopen(talk, "r");
                        send(confd[i], "send", strlen("send"), 0);
                        recv(confd[user], buf_rcv2, sizeof(buf_rcv2), 0);
                        if (strcmp(buf_rcv2, "NO") == 0)
                        {
                            sprintf(buf_snd4, " ");
                            send(confd[i], buf_snd4, strlen(buf_snd4), 0);
                            fclose(fp);
                            continue;
                        }
                        else
                        {
                            sprintf(buf_snd4, " ");
                            while (fgets(buf_snd3, 255, fp) != NULL)
                            {
                                strcat(buf_snd4, buf_snd3);
                            }
                            send(confd[i], buf_snd4, strlen(buf_snd4), 0);
                            fclose(fp);
                        }
                    }
                }
            }
            continue;
        }
        sprintf(buf_snd, "%s:%s", name, buf_rcv);
        printf("%s\n", buf_snd);
        for (int i = 0; i < count; i++)
        {
            if (i == index || 0 == confd[i])
            continue;
            send(confd[i], buf_snd, sizeof(buf_snd), 0);
        }
    }
}

int main(int argc, char **argv)
{
    for (int i = 0; i < 50; i++) table[i].state = 0;
    //創建通信對象
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0)
    {
        perror("socket");
        return -1;
    }
	printf("Default Port:9999,Default Address:localhost\n");
    //準備地址
    struct sockaddr_in addr = {AF_INET};
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t addr_len = sizeof(addr);
	//test
	/*int reuseaddr = 1;
    socklen_t reuseaddr_len;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, reuseaddr_len);*/
    //綁定
    int ret = bind(sockfd, (sockaddrp)&addr, addr_len);
    if (ret < 0 )
    {
        perror("bind");
        return -1;
    }


    //設置最大排隊數
    listen(sockfd, 50);

    int index = 0;

    while (count <= 50 )
    {
        confd[count] = accept(sockfd, (sockaddrp)&src_addr[count], &src_len);
	++count;
	index = count -1;
        //保存此次客戶端地址所在下標方便後續傳入

        pthread_t tid;
        int ret = pthread_create(&tid, NULL, broadcast, &index);
	printf("count=%d\n",count);
        if ( ret < 0 )
        {
            perror("pthread_create");
            return -1;
        }

    }

}
