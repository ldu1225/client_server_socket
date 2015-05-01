

//20113304 �̵���

//�迵��������

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>

void display();
void error_handling(char *message);

int peertcpSocket = -1;	// peer socket

int main(int argc, char **argv) {

	int tcpServ_sock;

	struct sockaddr_in tcpServer_addr;
	struct sockaddr_in tcpClient_addr;
	struct sockaddr_in newTcp_addr;

	int clnt_len;

	fd_set reads, temps;
	int fd_max;

	char command[1024];
	char cmdCpy[1024];	//�Է��� ���� �ӽ÷� ������ ����

	char *tcpport = NULL;
	char *userid = NULL;

	// NEED TO ADD SOME VARIABLES 
	char *cmd[4];	//�Է��� ������ ���� ����
	char *token;	//strtok�� ���� ����
	char strTemp[]=" ";		//strtok�� ���� ����
	struct hostent *hostp;		//����ü,������,�ּ�,�ٸ��̸�,����,ip�ּ�,ù��° IP�ּ�
	int comLength;		//�Է��� ����
	if(argc != 3){      //parameter�� 3���� �ƴϸ�!
		printf("Usage : %s <tcpport> <userid>\n", argv[0]);
		exit(1);
	}


	display();


	// NEED TO CREATE A SOCKET FOR TCP SERVER

	tcpServ_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(tcpServ_sock == -1 )		//������ ��������� �ʾ�����
		printf("Sock() error");	

	memset(&tcpServer_addr, 0, sizeof(tcpServer_addr));		//�ʱ�ȭ
	tcpServer_addr.sin_family=AF_INET;		//TCP�̿�
	tcpServer_addr.sin_addr.s_addr=htonl(INADDR_ANY);		//������ ���� IP �ּҼ���
	tcpServer_addr.sin_port=htons(atoi(argv[1]));		//��Ʈ�ѹ�



	// NEED TO bind
	if(bind(tcpServ_sock, (struct sockaddr *) &tcpServer_addr, sizeof(tcpServer_addr))==-1)//bind�Լ�(������ ���� IP�ּҿ� ���� ��Ʈ ��ȣ�� �����ϴ� ����)
		error_handling("bind() error");


	// NEED TO listen
	if(listen(tcpServ_sock,10)==-1)//������ ����, ��� ť�� ũ�� ����
		error_handling("listen() error");


	// initialize the select mask variables and set the mask with stdin and the tcp server socket

	FD_ZERO(&reads);	//�����ʱ�ȭ
	FD_SET(tcpServ_sock, &reads);	//���� ��ȣ�� 1�� tcpServ_sock ���
	FD_SET(fileno(stdin), &reads);	//���� ��ȣ�� 1�� fileno(stdin) ǥ���Է� ���
	tcpport = argv[1];		//��Ʈ�ѹ� ����
	userid = argv[2];		//ID����
	fd_max=tcpServ_sock;	//�ִ밪

	printf("%s> \n", userid);

	while(1) {
		int nfound;

		temps = reads;	

		nfound = select(fd_max+1, &temps, 0, 0, NULL);

		if(FD_ISSET(fileno(stdin), &temps)) {    //�Է��̺�Ʈ�� �߻��ߴ°�?
			// Input from the keyboard
			fgets(command, sizeof (command), stdin);
			FD_CLR(fileno(stdin), &temps);
			strcpy(cmdCpy,command);
			token = strtok(cmdCpy,strTemp);
			cmd[0] = token;

			if(!strcmp(cmd[0],"@talk")){    //Ŭ���̾�Ʈ
				cmd[1] = strtok(NULL,strTemp);
				cmd[2] = strtok(NULL,strTemp);
				hostp =gethostbyname(cmd[1]);

				memset((void *)&newTcp_addr, 0, sizeof(newTcp_addr));
				newTcp_addr.sin_family=AF_INET;
				newTcp_addr.sin_port=htons(atoi(cmd[2]));
				memcpy((void *)&newTcp_addr.sin_addr,hostp->h_addr,hostp->h_length);

				close(peertcpSocket);
				peertcpSocket = socket(PF_INET, SOCK_STREAM, 0);
				FD_SET(peertcpSocket, &reads);
				fd_max=peertcpSocket;
				if(connect(peertcpSocket,(struct sockaddr*)&newTcp_addr,sizeof newTcp_addr)==-1)
					error_handling("connect() Error");
			}
			else if(!strcmp(command,"@quit\n")){     //���������� �����̸�(Ŭ���̾�Ʈ)
				close(peertcpSocket);
				fd_max=peertcpSocket=tcpServ_sock;
				break;
			}else{      //(���� & Ŭ���̾�Ʈ)
				strcpy(cmdCpy,userid);
				strcat(cmdCpy," : "); 
				strcat(cmdCpy,command);
				write(peertcpSocket,cmdCpy,strlen(cmdCpy));
			}
			printf("%s> \n", userid);
		}
		else if(FD_ISSET(tcpServ_sock, &temps))    //���� ������� Ŭ���̾�Ʈ ��û���޴´�.
		{
			clnt_len = sizeof tcpClient_addr;
			peertcpSocket = accept(tcpServ_sock,(struct sockaddr*)&tcpClient_addr,&clnt_len);
			if(peertcpSocket == -1)
			{
				error_handling("accept() error");
			}
			FD_SET(peertcpSocket, &reads);
			fd_max=peertcpSocket;
			fprintf(stdout,"Connection form host %s, port %d, socket %d\n",inet_ntoa(tcpClient_addr.sin_addr),ntohs(tcpClient_addr.sin_port),peertcpSocket);//���
		}
		else if(FD_ISSET(peertcpSocket, &temps))
		{
			comLength = read(peertcpSocket, command, sizeof(command));
			if(comLength == -1){
				error_handling("read() error");
			}else if(comLength==0){
				fprintf(stdout,"Connection closed %d\n", peertcpSocket); 
				close(peertcpSocket);
				fd_max=peertcpSocket=tcpServ_sock;
			}else{
				write(1,command,comLength);
			}
		}

	}//while End
	close(tcpServ_sock);
	return 0;
}//main End

void display() {
	printf("Student ID : 20103355 \n");
	printf("Name : SHIN JI UNG  \n");
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n',stderr);
	exit(1);
}

