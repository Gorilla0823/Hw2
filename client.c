#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

typedef struct sockaddr *sockaddrp;
int sockfd;
char secret;
int mode = 0,sig = 0 ;
FILE *fp;
char filename[50] = "download_";
char fname[50];
/*void sig_handler(int signo)
{
	if(signo==SIGINT)
		sig=1;
}*/
void *recv_other(void *arg)
{
    while (1)
    {
        char buf[255] = {};
        char buf2[255] = {};
	    char buf4[255];
        int ret = recv(sockfd, buf, sizeof(buf), 0);
        if (strcmp("send", buf) == 0)
        {
            printf("Receive file YES/NO ?\n");
            strcat(filename, fname);
            strcat(filename, ".txt");
            printf("Accept? YES/NO\n");
            recv(sockfd, buf2, sizeof(buf2), 0);
            if (strcmp(buf2, " ") == 0)
                continue;
            fp = fopen(filename, "w+");
            fprintf(fp, "%s\n", buf2);
            fclose(fp);
            continue;
        }
        else
        {
            if ( ret < 0 )
            {
                perror("recv");
                return 0;
            }
            printf("%s\n", buf);
        }
    }
}

int main(int argc, char **argv)
{
    if ( argc != 2 )
    {
        perror("Invalid Argument");
        return -1;
    }
    //建立socket對象
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd < 0 )
    {
        perror("socket");
        return -1;
    }
    //準備連接地址
    struct sockaddr_in addr = {AF_INET};
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t addr_len = sizeof(addr);

    //連接
    int ret = connect(sockfd, (sockaddrp)&addr, addr_len);
    if ( ret < 0 )
    {
        perror("connect");
        return -1;
    }

    //發送名字
    char buf[255] = {};
    char name[255] = {};
    strcpy(name,argv[1]);
    strcpy(fname, name);
    ret = send(sockfd, name, strlen(name), 0);
    if (ret<0)
    {
        perror("connect");
        return -1;
    }

    //創建接收子線程
    pthread_t tid;
    ret = pthread_create(&tid, NULL, recv_other, NULL);

    if (0 > ret)
    {
        perror("pthread_create");
        return -1;
    }
    //循環發送
    while (1)
    {
        printf("%s:",name);
        scanf("%s", buf);
        /*if(!signal(SIGINT,sig_handler)== SIG_ERR){
	    sprintf(buf,"quit");
        }*/

        int ret = send(sockfd, buf, strlen(buf), 0);
        if ( ret < 0 )
        {
            perror("send");
            return -1;
        }
        //輸入quit退出
        if (strcmp("quit", buf)==0)
        {
            printf("%s,You've quit the chatroom.\n", name);
            return 0;
        }
    }
}
