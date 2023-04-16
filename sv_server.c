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

struct student_t{
	char MSSV[16];
	char Ho_ten[128];
	char Ngay_sinh[32];
	float CPA;
};

int main(int argc, char *argv[])
{
	if(argc != 3){
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

	char *log_file = argv[2];
    FILE *f = fopen(log_file, "a");
    char buf[sizeof(struct student_t)];
    int ret;

    ret = recv(client, buf, sizeof(buf), 0);

    time_t current_time;
    struct tm* local_time;
    char time_string[32];

    current_time = time(NULL);
    local_time = localtime(&current_time);

    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", local_time);
    struct student_t student;
    memcpy(&student, buf, sizeof(student));

    char log[1024];
    fprintf(f, "%s %s %s %s %s %.2f\n", inet_ntoa(client_addr.sin_addr), time_string, student.MSSV, student.Ho_ten, student.Ngay_sinh, student.CPA);

    fclose(f);

    close(client);
    close(listener);


}