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
	int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(9000);

	FILE *fp = fopen(argv[1], "r");
	char buf[1024];
	
	if(fp != NULL){
		size_t len = fread(buf, sizeof(char), sizeof(buf), fp);
		if(ferror(fp) != 0){
			fputs("Error reading file", stderr);
		}
		else
			buf[len++] = '\0';
	}

	int ret = sendto(sender, buf, strlen(buf), 0, (struct sockaddr*)&addr, sizeof(addr));

	fclose(fp);
	printf("%d bytes sent\n", ret);
}