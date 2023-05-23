#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#define MAX_LENGTH 256
char * standardize(char *word){
    int len = strlen(word);
    char new_word[256];
    int i = 0, idx = 0;
    while(i < len){
        if(isalpha(word[i])){
            new_word[idx] = word[i];
        }
        else{
            while(word[i+1] == ' ')
                i++;
            new_word[idx] = word[i];
        }
        i++;
        idx++;
    }
    int new_len = strlen(new_word);
    toupper(new_word[0]);
    for(int i=1; i<new_len; i++)
        tolower(new_word[i]);
    return new_word;
}

static char *strip_spaces (const char * word)
{
    int l;
    int n;
    const char * word_begins;
    const char * word_ends;

    l = strlen (word);
    n = l;
    word_begins = word;
    while (isspace (*word_begins)) {
        word_begins++;
        n--;
    }
    word_ends = word + l - 1;
    while (isspace (*word_ends)) {
        word_ends--;
        n--;
    }
    if (n == l) {
        return strdup (word);
    }
    else {
        char * copy;
        int i;

        copy = malloc (n + sizeof ((char) '\0'));
        if (! copy) {
            fprintf (stderr, "Out of memory.\n");
            exit (EXIT_FAILURE);
        }
        for (i = 0; i < n; i++) {
            copy[i] = word_begins[i];
        }
        copy[n] = '\0';
        return copy;
    }
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
                                if (strcmp(cmd, "client_id:") == 0)
                                {
                                    char *msg = "Ket noi duoc chap nhan.\n";
                                    send(client, msg, strlen(msg), 0);

                                    int k = 0;
                                    for (; k < num_users; k++)
                                        if (strcmp(user_accounts[k], user) == 0) break;
                                    
                                    if (k < num_users)
                                    {
                                        char *msg = "User da ton tai.\n";
                                        send(client, msg, strlen(msg), 0);
                                    }
                                    else
                                    {
                                        users[num_users] = client;
                                        user_accounts[num_users] = malloc(strlen(user) + 1);
                                        strcpy(user_accounts[num_users], user);
                                        num_users++;
                                        char msg[128];
                                        sprintf(msg, "Xin chao.Hien dang co %d client dang ket noi.\n", num_users);
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
                            for (int k = 0; k < num_users; k++)
                                if (users[k] == client)
                                    {
                                        char *new_buf = strip_spaces(buf);
                                        char *new_buf_2 = standardize(new_buf);
                                        send(client, new_buf_2, strlen(new_buf_2), 0);
                                    }
                        }
                    }
                }
            }
    }
    
    close(listener);    

    return 0;
}