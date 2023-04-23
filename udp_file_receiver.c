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

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Tham so khong hop le!\n");
		return 1;
	}
	int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(9000);

	bind(receiver, (struct sockaddr*)&addr, sizeof(addr));

	FILE *fp = fopen(argv[1], "wb");
	char buf[1024];
	int ret;

	ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
	fwrite(buf, 1, ret, fp);
	fclose(fp);


}