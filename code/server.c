#include <stdio.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <string.h>
#include <signal.h>

#include <sys/mman.h>
/*response ��� ����*/
char response_html[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

char response_jpg[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: image/jpg; \r\n\r\n";

char response_gif[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: image/gif; \r\n\r\n";

char response_not[] = "HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/html; \r\n\r\n"
"<html>\n404 NOT FOUND\n</html>";
/*wirte�� ���� ��� ���� ������ ����*/
int html_size = sizeof(response_html) - 1;
int jpg_size = sizeof(response_jpg) - 1;
int gif_size = sizeof(response_gif) - 1;
int not_size = sizeof(response_not) - 1;
int sd; //���� ����

//������ �Լ�
void *thread_func(void *ns){
	int tns = *(int *)ns; //
	char *response; // response�� ���ڿ� ����Ű�� ������
	int response_size; // response ũ��
	int fd; // ���� ��ũ����
	char filename[50]; // ���� �̸�
	char in[2000]; // Ŭ���̾�Ʈ ��û ���� ����
	char extra[10]; // Ȯ���� ��
	struct stat stat_buf; // ���� stat
	int n; // write��ȯ���� ���� ����
	int flag = 0; // ���� ���� ���� �Ǵ� �÷���

	if (read(tns, in, 2000) == -1) { //Ŭ���̾�Ʈ ��û ���ۿ� ����
    	perror("recv");
	}

	//sscanf(in, "GET %s",filename);
	strtok(in, " "); 
	sprintf(filename, "%s", strtok(NULL, " ")); //filename �̾ƿ�
	if(strlen(filename) < 2){ // iter1.jbnu.ac.kr:PORT �� ���� ���� ��
    	sprintf(filename, "%s", "/index.html"); //�⺻ �������� /index.html 
	}

	if((strstr(filename, "total.cgi")) != NULL){ // CGI ó������ �˻�
		int from = -1, to = -1;
		unsigned long long result; 
		char buf[512] = {'\0'};
		char temp[8] = {'\0'}; 
		sscanf(filename, "/total.cgi?from=%d&to=%d%s", &from, &to, temp); // from, to ����
		if(from > to || from < 0 || to < 0 || temp[0] != '\0'){ //from�� to���� ũ�ų�, from, to �� �����̰ų� ���ڵڿ� ���ڰ� ����  ���� ó��
			sprintf(buf, "%s<html>\n%s\n</html>",response_not, "\ncheck your argument, 'to' must be larger than 'from' or both positive number or too large value!!");
		}else{
				result = from!=1?((to*(1+to)/2) - (from*(from-1)/2)):(to*(1+to)/2);
				if(result < 0){ // ������� �����̸� overflow Ȥ�� ���ڰ� �ƴ� ���� ���� (-1,-1)�̱� ������ ���� ó��
						sprintf("buf, %s<html>\n%s\n</html>",response_not, "check your argument, too large value!!");	
				}else{ // buf�� ��� response ����
						sprintf(buf,"%s<html>\n%lu\n</html>",response_html, result);
			}
		}
		write(tns, buf, 512); // Ŭ���̾�Ʈ���� ��� ����

	}else{ //CGI�� �ƴϸ�

    	if(access(filename+1, R_OK) == -1 || (fd = open(filename+1 ,O_RDONLY)) == -1){ // ���� ���� ���� �˻�
    		response = response_not; // 404 not found ���� ����
    		response_size = not_size;
    		flag = 1;
    }else{
    	fstat(fd, &stat_buf); //sendfile�� filesize�� �˱� ���� ó��
    	strtok(filename, ".");
		sprintf(extra, "%s",strtok(NULL, ".")); // Ȯ���� �˻�

	    switch(extra[0]){ // Ȯ���� ���� ó��
	    	case 'h': // html������ ��
	    		response = response_html;
	    		response_size = html_size;
	    		break;
	    	case 'j': // jpg ���� �� ��
	    		response = response_jpg;
	    		response_size = jpg_size;
	    		break;
	    	case 'g': // gif ���� �� ��
	    		response = response_gif;
	    		response_size = gif_size;
    		break;
    	default: // 404 not found �� ��
    			response = response_not;
    			response_size = not_size;
    		break;
    	}
    }

    if((n = write(tns, response, response_size)) != response_size){ //��� ����
		perror("response write");
	}
	/*�������3*/
 	if((flag == 0 && sendfile(tns, fd, NULL, stat_buf.st_size)) == -1){ //sendfile �� ��û ���� ����
 		perror("sendfile");
 	}
 	/*�ּ�3 ��*/
 	close(fd); //file ��ũ���� �ݱ�
 }
  	close(tns); // ������ ���� �ݱ�
}

int main(int argc, char const *argv[]){
	int optvalue=1; //���� �ɼ� ��
	pthread_t tid; //������ id
	struct sockaddr_in sin, cli; //���� ����ü
	int clientlen = sizeof(cli); 

	char *dir = argv[1];	
	char slash[] = "/"; 
	int PORTNUM;

	if(argc == 3){ //argument �˻�, ������, ��Ʈ��ȣ �Է�
			chdir(strtok(dir, slash)); //���丮 ü����
			PORTNUM = atoi(argv[2]); //��Ʈ ��ȣ
	}else{
		printf("check your argument, <directory port>\n");
		exit(1);
	}
	signal(SIGPIPE, SIG_IGN); // broken pipe ����

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // TCP socket ��ũ���� ����
		perror("socket");
		exit(1);
	}
	/*������� �ּ�1*/
	if(setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &optvalue, sizeof(optvalue)) == -1){ // Nagle �˰��� OFF
		perror("NODELAY");
	}
	/*�ּ� 1 ��*/
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)) == -1){ // ��������� �ٷ� �ٽ� bind�� �� �ֵ��� �ϴ� �ɼ�
		perror("NODELAY");
	}

	//���� ���� ����
	memset((char *)&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET; //TCP���
	sin.sin_port = htons(PORTNUM); //��Ʈ ��ȣ����
	sin.sin_addr.s_addr = INADDR_ANY; //��� ������ ���� ���

	if(bind(sd, (struct sockaddr *)&sin, sizeof(sin))){ //socket binding
		perror("bind");
		exit(1);
	}

	if(listen(sd, 5)){ //listening ���·� accept�� ��ٸ�
		perror("listen");
		exit(1);		
	}

	while(1){
		int *ns = (int*)malloc(sizeof(int));
		if((*ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1){ //Ŭ���̾�Ʈ ������ �Ǹ�
			perror("accept");
			exit(1);
		}
		if((pthread_create(&tid, NULL, thread_func, (void*)ns)) != 0){ // ������ ����
			perror("Thread Create Error");
		}
		/*���� ��� �ּ�2*/
		pthread_detach(tid); //������ �и�
		/*�ּ� 2 ��*/
	}

	close(sd); //���� �ݱ�

	return 0;
}

