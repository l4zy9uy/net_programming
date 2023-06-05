#include<stdio.h>
#include <unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>


int main(int argc, char *argv[])
{
	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listener == -1){
		perror("connect() failed");
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr))){
		perror("bind () failed");
		return 1;
	}
	if(listen(listener, 5)){
		perror("listen() failed");
		return 1;
	}

    int num_process = 8;
    char buf[256];
	for(int i = 0; i < num_process; i++){
        if(fork() == 0){
            while(1){
                int client = accept(listener, NULL, NULL);
                printf("New client connected: %d\n", client);
                
                int ret = recv(client, buf, sizeof(buf), 0);
                buf[ret] = 0;
                puts(buf);

                char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";

                send(client, msg, strlen(msg), 0);

                close(client);
            }
        }
    }
    wait(NULL);
}