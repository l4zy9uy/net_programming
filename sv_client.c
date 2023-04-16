#include<stdio.h>
#include <unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h>
#include<errno.h>
#include<string.h>

struct student_t{
	char MSSV[16];
	char Ho_ten[128];
	char Ngay_sinh[32];
	float CPA;
};

int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("Tham so khong hop le!\n");
		return 1;
	}

	struct student_t student;

	printf("Nhap MSSV: ");
	fgets(student.MSSV, 16, stdin);
	student.MSSV[strcspn(student.MSSV, "\n")] = 0;
	printf("Nhap ho va ten: ");
	fgets(student.Ho_ten, 128, stdin);
	student.Ho_ten[strcspn(student.Ho_ten, "\n")] = 0;
	printf("Nhap ngay sinh: ");
	fgets(student.Ngay_sinh, 32, stdin);
	student.Ngay_sinh[strcspn(student.Ngay_sinh, "\n")] = 0;
	printf("Nhap CPA: ");
	scanf("%f", &student.CPA);
	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	char buf[sizeof(student)];
	memcpy(buf, &student, sizeof(student));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(atoi(argv[2]));

	int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		perror("connect() failed!\n");
		return 1;
	}
	send(client, buf, sizeof(buf), 0);
	
	close(client);
}