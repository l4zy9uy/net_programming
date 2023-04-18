#include<stdio.h>
#include <unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h>
#include<errno.h>
#include<string.h>

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("Tham so khong hop le");
		return 1;
	}

	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(9000);

	int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		perror("connect() failed!\n");
		return 1;
	}

	char buf[4096];
	FILE *f = fopen(argv[1], "r");
	fgets(buf, sizeof(buf), f);
	printf("file content: %s", buf);

	send(client, buf, sizeof(buf), 0);
	
	close(client);
}