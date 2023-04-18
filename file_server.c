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

int countString(const char *haystack, const char *needle){
    int count = 0;
    const char *tmp = haystack;
    while(tmp = strstr(tmp, needle))
    {
        count++;
        tmp++;
    }
    return count;
}

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

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);

    char buf[4096];
    int ret;
    char *pattern = "0123456789";

    ret = recv(client, buf, sizeof(buf), 0);

    printf("So lan xuat hien xau ki tu: %d", countString(buf, pattern));
    close(client);
    close(listener);


}