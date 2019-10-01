/**********************************************************************
 * Copyright (c) 2019
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include "types.h"
#include "parser.h"

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
/**
 * String used as the prompt (see @main()). You may change this to
 * change the prompt */
static char __prompt[MAX_TOKEN_LEN] = "$";

/**
 * Time out value. It's OK to read this value, but ** DO NOT CHANGE
 * DIRECTLY **. Instead, use @set_timeout() function below.
 */
static unsigned int __timeout = 2;

static void set_timeout(unsigned int timeout)
{
	__timeout = timeout;

	if (__timeout == 0) {
		fprintf(stderr, "Timeout is disabled\n");
	} else {
		fprintf(stderr, "Timeout is set to %d second%s\n",
				__timeout,
				__timeout >= 2 ? "s" : "");
	}
}
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/

/***********************************************************************
 * timeout_handler()
 *
 * DESCRIPTION
 * 	when receive alarm signal, kill child process
 */
char name[1024];
char* childname = name;
pid_t pid;
int status;
bool timer_set = false;

struct sigaction act;

void timeout_handler(){
	kill(pid,SIGKILL); // 자식 죽이기
	fprintf(stderr,"%s is timed out\n",childname); // 죽은 자식 출력
	memset(name,0,1024); // 자식이름 담은 배열 초기화

}

/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
static int run_command(int nr_tokens, char *tokens[])
{
	act.sa_handler = timeout_handler;
	act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGALRM,&act,0);

	if (strncmp(tokens[0], "exit", strlen("exit")) == 0) {
		/* information -
			종료 명령
		*/
		return 0;
	}else if(strncmp(tokens[0], "prompt", strlen("prompt")) == 0){
		/* informaion -
			프롬프트
			static char __prompt[MAX_TOKEN_LEN] = "$"; -> original
		*/
		strcpy(__prompt,tokens[1]);
	}else if(strncmp(tokens[0],"cd",strlen("cd"))==0){
		/* information -
			cd 명령은 fork - exec로 하면 
		   	자식프로세스가 이동하고 끝나기
		   	때문에 따로 구현해줘야 한다.
		*/
		char buffer[1024];
		char * p = buffer;
		if(strncmp(tokens[1],"~",strlen("~"))==0){ // home 으로 이동하는 루틴
			p = getenv("HOME");
			if(chdir(p)==-1){
				fprintf(stderr, "No such file or directory\n");
			}
		}else{
			if(chdir(tokens[1])==-1){
				fprintf(stderr, "No such file or directory\n");
			}
			/* information -
				chdir함수는 디렉토리를 바꿀 때 이용하는 함수이다.
				이동할 위치를 포인터 형태로 넘기면
				포인터가 가리키고 있는 위치로 이동시키는 역할을 한다.

				getenv함수는 환경변수를 얻어오는 역할을 한다.
				예로 환경변수 HOME은 사용자의 홈 디렉토리 위치를 포함
				한다.

				환경변수란? 
				프로세스가 컴퓨터에서 동작하는 방식에 영향을 미치는, 
				동적인 값들의 모임
			*/
		}
	}else if(strncmp(tokens[0],"for",strlen("for"))==0){
		/* information -
		for 명령어 실행 코드
			algorithm ->
			1. for로 시작하는 명령임을 인식한다.
			2. 숫자가 나오는 것을 계속 곱한다.(중첩 루프)
			3. 숫자도 for도 아닌 문자열이 나오는 것을 인식하고, command 배열에 이것을 담아둔다.
			4. cd 혹은 기타 명령어 일 것이므로 그에 맞게 실행시켜 준다.
		*/
		int loop = 1; // loop counter
		int index = 1; // tokens array index
		int t = 0; // command array index
		char* command[1024]; // command array for execute command

		while(tokens[index] != NULL){
			if(atoi(tokens[index]) != 0){ // loop count code
				loop *= atoi(tokens[index]);
			}else{ // not number
				if(strncmp(tokens[index],"for",strlen("for") != 0)){ // not for
					command[t++] = tokens[index]; // command array에 실행시킬 명령어를 담아둔다.
				}
			}
			index++; // 인덱스 증가
		}
		
		while(loop--){ // 이제 반복시킨다.
			if(strncmp(command[0],"cd",strlen("cd"))==0){ // cd 일 때,
				char buffer[1024];
				char * p = buffer;
				if(strncmp(command[1],"~",strlen("~"))==0){
					if(chdir(p)==-1){
					fprintf(stderr, "No such file or directory\n");
					}
				}else{
					if(chdir(command[1])==-1){
					fprintf(stderr, "No such file or directory\n");
					}
				}
			}else{ // cd 제외 명령어 일 때,
				if ( (pid=fork()) == 0 ){
					if(execvp(command[0],command)<0){
						fprintf(stderr, "No such file or directory\n");
					}
					exit(0);
				}else{
        			waitpid(pid,&status,0);
				}
			}
		}
		memset(command,0,1024); // command 배열 초기화
		//printf("%d\n",loop);
		//printf("%s\n",command);
	}else if(strncmp(tokens[0],"timeout",strlen("timeout"))==0){
		/* information - 
		timeout 명령어
		*/
		if(tokens[1] == NULL){
			fprintf(stderr,"Current timeout is 0 second\n");
		}else{
			set_timeout(atoi(tokens[1])); // 타이머 맞추기
			if(atoi(tokens[1])){ // 시간이 0이 아니면..
				// fprintf(stderr,"타이머 설정\n");
				timer_set = true; // 타이머 동작
			}else{
				timer_set = false; // 0이면 타이머 해제
			}
		}
	}else{
		if ( (pid=fork()) == 0 ){
			/* information -
			fork함수를 이용하여 자식프로세스 생성시켰다.
			자식은 쉘일 필요가 없고 사용자의 명령어를 실행,
			부모와는 다른 역할을 하기 위해 exec family가
			필요하다.
			exec family 중 execvp를 사용하였다.
			man execvp 참조..
			execvp(파일/명령어,argv);
			*/
			if(execvp(tokens[0],tokens)<0){
				fprintf(stderr, "No such file or directory\n");
				kill(getpid(),SIGKILL); // 비정상 종료시 exit문으로 가지 않기때문에 종료되지 않는다 비정상종료시켜야된다.
				// abort(); ->  이것도 가능
				// assert(0); -> 이것도 가능
			}
			exit(0);
		}else{
			/* information -
			부모는 자식이 끝나기를 기다린다.
			그리고 자식의 리소스를 반환한다
			리소스 반환시키려면 wait함수가 사용되야하는데
			누가 죽었는지 알기위해서는 waitpid를 사용하고
			자식하나를 기다리려면 wait를 사용해도 무방한데
			자식이 여러개인 경우는 waitpid함수를 사용하는 것이 좋다
			왜냐하면 wait함수는 누가 죽엇는지 모른다.
			*/
			childname = tokens[0];
			if(timer_set){
				alarm(__timeout);
			}
			
        	waitpid(pid,&status,2); // 자식 종료 전까지는 안끝나니까 이게 끝나서 밑줄로 내려갓다는 의미는 자식이 종료됫음을 의미
			
			alarm(0); // 자식이 먼저 종료됫으므로 알람 취소
		}
	}
	return 1;
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

static bool __verbose = true;
static char *__color_start = "[0;31;40m";
static char *__color_end = "[0m";

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	if (__verbose)
		fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);

	while (fgets(command, sizeof(command), stdin)) {	
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		if (parse_command(command, &nr_tokens, tokens) == 0)
			goto more; /* You may use nested if-than-else, however .. */

		ret = run_command(nr_tokens, tokens);
		if (ret == 0) {
			break;
		} else if (ret < 0) {
			fprintf(stderr, "Error in run_command: %d\n", ret);
		}

more:
		if (__verbose)
			fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
