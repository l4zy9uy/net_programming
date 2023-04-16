#include<stdio.h>
#include <unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>


int main(int argc, char *argv[])
{
	if(argc != 4){
		printf("Tham so khong hop le!");
		return 1;
	}
	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listener == -1){
		perror("connect() failed");
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(atoi(argv[1]));

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

	printf("Accepted socket %d from IP: %s:%d\n", 
        client,
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));

	char *file_send = argv[2], *file_recv = argv[3];

    FILE *f1 = fopen(file_send, "rb");
    char buf[2048];
    int ret;

    while (!feof(f1))
    {
        ret = fread(buf, 1, sizeof(buf), f1);
        if (ret <= 0)
            break;
    }
    send(client, buf, strlen(buf), 0);

    FILE *f2 = fopen(file_recv, "wb");

    int len = strlen(file_send);
    send(client, &len, sizeof(len), 0);
    send(client, file_send, strlen(file_send), 0);


    memset(buf, 0, sizeof(buf));
    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f2);
    }
    fclose(f1);
    fclose(f2);

    close(client);
    close(listener);


}