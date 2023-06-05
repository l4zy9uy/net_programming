#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define MAX_LENGTH 64
#define DASH "-"



void signalHandler(int signo){
    int stat;
    printf("signo = %d\n", signo);
    int pid = wait(&stat);
    printf("child %d terminated.\n", pid);
    return;
}

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9090);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }
    signal(SIGCHLD, signalHandler);

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    char buf[256];

    int users[64];      // Mang socket client da dang nhap
    char *user_accounts[64]; // Mang luu tru id cua client da dang nhap
    int num_users = 0;  // So client da dang nhap

    time_t currentTime = time(NULL);
    struct tm* localTime = localtime(&currentTime);

    char timebuf[20];
    while (1)
    {
        fdtest = fdread;

        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            break;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    if(fork() == 0){
                        // Chap nhan ket noi
                        int client = accept(listener, NULL, NULL);
                        if (client < FD_SETSIZE)
                        {
                            printf("New client connected: %d\n", client);
                            FD_SET(client, &fdread);
                        }
                        else
                        {
                            printf("Too many connections.\n");
                            close(client);
                        }
                    }
                }
                else
                {
                    // Nhan du lieu
                    int ret = recv(i, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        printf("Client %d disconnected.\n", i);
                        close(i);
                        FD_CLR(i, &fdread);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", i, buf);

                        // Kiem tra trang thai dang nhap cua client
                        int client = i;

                        int j = 0;
                        for (; j < num_users; j++)
                            if (users[j] == client) break;

                        if (j == num_users)
                        {
                            // Chua dang nhap
                            // Xu ly cu phap yeu cau dang nhap
                            char format[32], cmd[32], tmp[32];
                            ret = sscanf(buf, "%s%s%s", cmd, format, tmp);
                            if (ret == 2)
                            {
                                if (strcmp(cmd, "GET_TIME") == 0)
                                {
                                    char time_format[16];
                                    int check = 1;
                                    if(format == "[dd/mm/yyyy]")
                                        strcpy(time_format, "%d/%m/%Y");
                                    else if(format == "[dd/mm/yy]")
                                        strcpy(time_format, "%d/%m/%y");
                                    else if(format == "[mm/dd/yyyy]")
                                        strcpy(time_format, "%m/%d/%Y");
                                    else if(format ==  "[mm/dd/yy]")
                                        strcpy(time_format, "%m/%d/%y");
                                    else
                                        check = 0;

                                    if(check == 1){
                                        strftime(timebuf,  sizeof(timebuf), time_format, localTime);
                                        char *msg = strcat(timebuf, "\n"); 
                                        send(client, msg, strlen(msg), 0);
                                    }
                                    else{
                                        char *msg = "Cu phap khong hop le.\n";
                                        send(client, msg, strlen(msg), 0);
                                    }

                                }
                                else
                                {
                                    char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                    send(client, msg, strlen(msg), 0);
                                }
                            }
                            else
                            {
                                char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                send(client, msg, strlen(msg), 0);
                            }
                        }
                    }
                }
            }
    }
    
    close(listener);    
    killpg(0, SIGKILL);
    return 0;
}