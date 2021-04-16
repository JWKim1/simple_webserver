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
/*response 헤더 정보*/
char response_html[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

char response_jpg[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: image/jpg; \r\n\r\n";

char response_gif[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: image/gif; \r\n\r\n";

char response_not[] = "HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/html; \r\n\r\n"
"<html>\n404 NOT FOUND\n</html>";
/*wirte를 위한 헤더 정보 사이즈 정의*/
int html_size = sizeof(response_html) - 1;
int jpg_size = sizeof(response_jpg) - 1;
int gif_size = sizeof(response_gif) - 1;
int not_size = sizeof(response_not) - 1;
int sd; //소켓 선언

//스레드 함수
void *thread_func(void *ns){
	int tns = *(int *)ns; //
	char *response; // response할 문자열 가리키는 포인터
	int response_size; // response 크기
	int fd; // 파일 디스크립터
	char filename[50]; // 파일 이름
	char in[2000]; // 클라이언트 요청 저장 버퍼
	char extra[10]; // 확장자 명
	struct stat stat_buf; // 파일 stat
	int n; // write반환값을 받을 변수
	int flag = 0; // 파일 존재 여부 판단 플래그

	if (read(tns, in, 2000) == -1) { //클라이언트 요청 버퍼에 저장
    	perror("recv");
	}

	//sscanf(in, "GET %s",filename);
	strtok(in, " "); 
	sprintf(filename, "%s", strtok(NULL, " ")); //filename 뽑아옴
	if(strlen(filename) < 2){ // iter1.jbnu.ac.kr:PORT 로 접속 했을 시
    	sprintf(filename, "%s", "/index.html"); //기본 페이지인 /index.html 
	}

	if((strstr(filename, "total.cgi")) != NULL){ // CGI 처리인지 검사
		int from = -1, to = -1;
		unsigned long long result; 
		char buf[512] = {'\0'};
		char temp[8] = {'\0'}; 
		sscanf(filename, "/total.cgi?from=%d&to=%d%s", &from, &to, temp); // from, to 대입
		if(from > to || from < 0 || to < 0 || temp[0] != '\0'){ //from이 to보다 크거나, from, to 가 음수이거나 숫자뒤에 문자가 들어가면  에러 처리
			sprintf(buf, "%s<html>\n%s\n</html>",response_not, "\ncheck your argument, 'to' must be larger than 'from' or both positive number or too large value!!");
		}else{
				result = from!=1?((to*(1+to)/2) - (from*(from-1)/2)):(to*(1+to)/2);
				if(result < 0){ // 결과값이 음수이면 overflow 혹은 숫자가 아닌 값이 들어간것 (-1,-1)이기 때문에 에러 처리
						sprintf("buf, %s<html>\n%s\n</html>",response_not, "check your argument, too large value!!");	
				}else{ // buf에 결과 response 저장
						sprintf(buf,"%s<html>\n%lu\n</html>",response_html, result);
			}
		}
		write(tns, buf, 512); // 클라이언트에게 결과 전송

	}else{ //CGI가 아니면

    	if(access(filename+1, R_OK) == -1 || (fd = open(filename+1 ,O_RDONLY)) == -1){ // 파일 존재 여부 검사
    		response = response_not; // 404 not found 정보 저장
    		response_size = not_size;
    		flag = 1;
    }else{
    	fstat(fd, &stat_buf); //sendfile의 filesize를 알기 위한 처리
    	strtok(filename, ".");
		sprintf(extra, "%s",strtok(NULL, ".")); // 확장자 검사

	    switch(extra[0]){ // 확장자 파일 처리
	    	case 'h': // html파일일 때
	    		response = response_html;
	    		response_size = html_size;
	    		break;
	    	case 'j': // jpg 파일 일 떄
	    		response = response_jpg;
	    		response_size = jpg_size;
	    		break;
	    	case 'g': // gif 파일 일 때
	    		response = response_gif;
	    		response_size = gif_size;
    		break;
    	default: // 404 not found 일 때
    			response = response_not;
    			response_size = not_size;
    		break;
    	}
    }

    if((n = write(tns, response, response_size)) != response_size){ //헤더 전송
		perror("response write");
	}
	/*성능향상3*/
 	if((flag == 0 && sendfile(tns, fd, NULL, stat_buf.st_size)) == -1){ //sendfile 로 요청 파일 전송
 		perror("sendfile");
 	}
 	/*주석3 끝*/
 	close(fd); //file 디스크립터 닫기
 }
  	close(tns); // 스레드 소켓 닫기
}

int main(int argc, char const *argv[]){
	int optvalue=1; //소켓 옵션 값
	pthread_t tid; //스레드 id
	struct sockaddr_in sin, cli; //소켓 구조체
	int clientlen = sizeof(cli); 

	char *dir = argv[1];	
	char slash[] = "/"; 
	int PORTNUM;

	if(argc == 3){ //argument 검사, 폴더명, 포트번호 입력
			chdir(strtok(dir, slash)); //디렉토리 체인지
			PORTNUM = atoi(argv[2]); //포트 번호
	}else{
		printf("check your argument, <directory port>\n");
		exit(1);
	}
	signal(SIGPIPE, SIG_IGN); // broken pipe 방지

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // TCP socket 디스크립터 생성
		perror("socket");
		exit(1);
	}
	/*성능향상 주석1*/
	if(setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &optvalue, sizeof(optvalue)) == -1){ // Nagle 알고리즘 OFF
		perror("NODELAY");
	}
	/*주석 1 끝*/
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)) == -1){ // 강제종료시 바로 다시 bind할 수 있도록 하는 옵션
		perror("NODELAY");
	}

	//소켓 정보 정의
	memset((char *)&sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET; //TCP통신
	sin.sin_port = htons(PORTNUM); //포트 번호정의
	sin.sin_addr.s_addr = INADDR_ANY; //모든 아이피 접근 허용

	if(bind(sd, (struct sockaddr *)&sin, sizeof(sin))){ //socket binding
		perror("bind");
		exit(1);
	}

	if(listen(sd, 5)){ //listening 상태로 accept를 기다림
		perror("listen");
		exit(1);		
	}

	while(1){
		int *ns = (int*)malloc(sizeof(int));
		if((*ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1){ //클라이언트 접속이 되면
			perror("accept");
			exit(1);
		}
		if((pthread_create(&tid, NULL, thread_func, (void*)ns)) != 0){ // 스레드 생성
			perror("Thread Create Error");
		}
		/*성능 향상 주석2*/
		pthread_detach(tid); //스레드 분리
		/*주석 2 끝*/
	}

	close(sd); //소켓 닫기

	return 0;
}

