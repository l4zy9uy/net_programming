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
	if(argc != 3){
		printf("Tham so khong hop le!");
		return 1;
	}
	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		perror("connect() failed!\n");
		return 1;
	}

	char msg[2048],buf[2048];

	fgets(msg, 2048, stdin);

	send(client, msg, strlen(msg), 0);
	ret = recv(client, buf, sizeof(buf), 0);
	if(ret <= 0)
		return 1;
	if(ret < sizeof(buf))
		buf[ret] = 0;
	printf("Receive from server: %s\n", buf);

	close(client);
}