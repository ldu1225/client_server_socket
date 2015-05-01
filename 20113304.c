

//20113304 이동욱

//김영만교수님

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
	char cmdCpy[1024];	//입력한 것을 임시로 저장할 변수

	char *tcpport = NULL;
	char *userid = NULL;

	// NEED TO ADD SOME VARIABLES 
	char *cmd[4];	//입력을 나누기 위한 변수
	char *token;	//strtok을 위한 변수
	char strTemp[]=" ";		//strtok을 위한 변수
	struct hostent *hostp;		//구조체,도메인,주소,다른이름,길이,ip주소,첫번째 IP주소
	int comLength;		//입력의 길이
	if(argc != 3){      //parameter가 3개아 아니면!
		printf("Usage : %s <tcpport> <userid>\n", argv[0]);
		exit(1);
	}


	display();


	// NEED TO CREATE A SOCKET FOR TCP SERVER

	tcpServ_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(tcpServ_sock == -1 )		//소켓이 만들어지지 않았을때
		printf("Sock() error");	

	memset(&tcpServer_addr, 0, sizeof(tcpServer_addr));		//초기화
	tcpServer_addr.sin_family=AF_INET;		//TCP이용
	tcpServer_addr.sin_addr.s_addr=htonl(INADDR_ANY);		//서버의 지역 IP 주소설정
	tcpServer_addr.sin_port=htons(atoi(argv[1]));		//포트넘버



	// NEED TO bind
	if(bind(tcpServ_sock, (struct sockaddr *) &tcpServer_addr, sizeof(tcpServer_addr))==-1)//bind함수(서버의 지역 IP주소와 지역 포트 번호를 결정하는 역할)
		error_handling("bind() error");


	// NEED TO listen
	if(listen(tcpServ_sock,10)==-1)//소켓을 지정, 대기 큐의 크기 지정
		error_handling("listen() error");


	// initialize the select mask variables and set the mask with stdin and the tcp server socket

	FD_ZERO(&reads);	//변수초기화
	FD_SET(tcpServ_sock, &reads);	//소켄 번호를 1로 tcpServ_sock 등록
	FD_SET(fileno(stdin), &reads);	//소켓 번호를 1로 fileno(stdin) 표준입력 등록
	tcpport = argv[1];		//포트넘버 저장
	userid = argv[2];		//ID저장
	fd_max=tcpServ_sock;	//최대값

	printf("%s> \n", userid);

	while(1) {
		int nfound;

		temps = reads;	

		nfound = select(fd_max+1, &temps, 0, 0, NULL);

		if(FD_ISSET(fileno(stdin), &temps)) {    //입력이벤트가 발생했는가?
			// Input from the keyboard
			fgets(command, sizeof (command), stdin);
			FD_CLR(fileno(stdin), &temps);
			strcpy(cmdCpy,command);
			token = strtok(cmdCpy,strTemp);
			cmd[0] = token;

			if(!strcmp(cmd[0],"@talk")){    //클라이언트
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
			else if(!strcmp(command,"@quit\n")){     //빠져나가는 문장이면(클라이언트)
				close(peertcpSocket);
				fd_max=peertcpSocket=tcpServ_sock;
				break;
			}else{      //(서버 & 클라이언트)
				strcpy(cmdCpy,userid);
				strcat(cmdCpy," : "); 
				strcat(cmdCpy,command);
				write(peertcpSocket,cmdCpy,strlen(cmdCpy));
			}
			printf("%s> \n", userid);
		}
		else if(FD_ISSET(tcpServ_sock, &temps))    //아직 서버라면 클라이언트 요청을받는다.
		{
			clnt_len = sizeof tcpClient_addr;
			peertcpSocket = accept(tcpServ_sock,(struct sockaddr*)&tcpClient_addr,&clnt_len);
			if(peertcpSocket == -1)
			{
				error_handling("accept() error");
			}
			FD_SET(peertcpSocket, &reads);
			fd_max=peertcpSocket;
			fprintf(stdout,"Connection form host %s, port %d, socket %d\n",inet_ntoa(tcpClient_addr.sin_addr),ntohs(tcpClient_addr.sin_port),peertcpSocket);//출력
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

