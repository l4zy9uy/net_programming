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

struct info{
	char COMPUTER_NAME[32];
	char DISK_NAME[32];
	unsigned short int DISK_NUMBER;
	unsigned short int DISK_SIZE[32];
};


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

    char buf[sizeof(struct info)];
    int ret;

    ret = recv(client, buf, sizeof(buf), 0);

    struct info user;
    memcpy(&user, buf, sizeof(user));

    printf("+ Ten may tinh: %s\n", user.COMPUTER_NAME);
    printf("+ So o dia: %d\n", user.DISK_NUMBER);
    for(int i=0; i<user.DISK_NUMBER; i++){
    	printf("	%c - %dGB\n", user.DISK_NAME[i], user.DISK_SIZE[i]);
    }

    close(client);
    close(listener);


}