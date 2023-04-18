#include<stdio.h>
#include <unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h>
#include<errno.h>
#include<string.h>

struct info{
	char COMPUTER_NAME[32];
	char DISK_NAME[32];
	unsigned short int DISK_NUMBER;
	unsigned short int DISK_SIZE[32];
};

int main(int argc, char *argv[])
{
	struct info user;

	printf("Ten may tinh: ");
	scanf("%s", user.COMPUTER_NAME);
	getchar();
	printf("So luong o dia: ");
	scanf("%hd", &user.DISK_NUMBER);
	getchar();
	for(int i=0; i<user.DISK_NUMBER; i++){
		printf("Ten o dia va dung luong: ");
		scanf("%c %hd", &user.DISK_NAME[i], &user.DISK_SIZE[i]);
		getchar();
	}

	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	char buf[sizeof(user)];
	memcpy(buf, &user, sizeof(user));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(9000);

	int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		perror("connect() failed!\n");
		return 1;
	}
	send(client, buf, sizeof(buf), 0);
	
	close(client);
}