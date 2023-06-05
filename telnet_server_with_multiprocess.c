#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define MAX_LENGTH 64
#define DASH "-"

int checkCredentials(const char* account, const char* password) {
    FILE* file = fopen("data.txt", "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 0;
    }

    char line[MAX_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("user: %s\n", line);
        
        line[strcspn(line, "\n")] = '\0';  // Remove trailing newline character

        char stored_account[MAX_LENGTH];
        char stored_password[MAX_LENGTH];

        char* token = strtok(line, DASH);
        strcpy(stored_account, token);

        token = strtok(NULL, DASH);
        strcpy(stored_password, token);

        if (strcmp(account, stored_account) == 0 && strcmp(password, stored_password) == 0) {
            fclose(file);
            return 1;  // Credentials are correct
        }
    }

    fclose(file);
    return 0;  // Credentials are incorrect
}

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
                            char user[32], cmd[32], tmp[32], pass[32];
                            ret = sscanf(buf, "%s%s%s", cmd, user, tmp);
                            if (ret == 2)
                            {
                                if (strcmp(cmd, "user:") == 0)
                                {
                                    int ret = recv(i, buf, sizeof(buf), 0);
                                    ret = sscanf(buf, "%s%s%s", cmd, pass, tmp);
                                    if(strcmp(cmd, "pass:") == 0){
                                        if(checkCredentials(user, pass))
                                        {
                                            char *msg = "Tai khoan va mat khau hop le.\n";
                                            send(client, msg, strlen(msg), 0);

                                            int k = 0;
                                            for (; k < num_users; k++)
                                                if (strcmp(user_accounts[k], user) == 0) break;
                                            
                                            if (k < num_users)
                                            {
                                                char *msg = "tai khoan da ton tai. Yeu cau nhap lai.\n";
                                                send(client, msg, strlen(msg), 0);
                                            }
                                            else
                                            {
                                                users[num_users] = client;
                                                user_accounts[num_users] = malloc(strlen(user) + 1);
                                                strcpy(user_accounts[num_users], user);
                                                num_users++;
                                            }
                                        }
                                        else{
                                            char *msg = "Khong ton tai tai khoan mat khau.\n";
                                            send(client, msg, strlen(msg), 0);
                                        }
                                    }
                                    else{
                                        char *msg = "Nhap sai. Yeu cau nhap lai tu dau.\n";
                                        send(client, msg, strlen(msg), 0);
                                    }                                    
                                }
                                else
                                {
                                    char *msg = "Nhap sai. Yeu cau nhap lai tai khoan.\n";
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
                            // Da dang nhap
                            char tmp[256];
                            if(fork() == 0){
                                if(buf[strlen(buf) - 1] == '\n')
                                    buf[strlen(buf) - 1] = '\0';

                                sprintf(tmp, "%s > out.txt", buf);
                                int ret = system(tmp);
                            }
                            else{
                                if(ret == 0){
                                    FILE *f = fopen("out.txt", "rb");
                                    while(!feof(f)){
                                        ret = fread(tmp, 1, sizeof(tmp), f);
                                        if(ret <= 0)
                                            break;
                                        send(client, tmp, ret, 0);
                                    }
                                    fclose(f);
                                }
                                else{
                                    char *msg = "Lenh khong thuc hien duoc";
                                    send(client, tmp, ret, 0);
                                }
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