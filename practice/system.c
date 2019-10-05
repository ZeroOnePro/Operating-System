#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

/* pwd
int main(int argc,char* argv[])
{
    char buffer[1024];
	getcwd(buffer,1024);
	printf("%s\n",buffer);

    return 0;
}
*/

/* fork example
int x = 3;

int main(int argc, const char* argv[]){
    while(x--){
        fork();
        printf("%d\n",x);
    }
    return 0;
}
*/

/* os project1 practice
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

void timeout_handler(){
    
}

int main(int argc, const char* argv[]){
    int status;
    struct sigaction act;
    act.sa_handler = set_timeout;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM,&act,0);
    pid_t pid;
    if((pid=fork())==0){
        sleep(20);
        printf("sex\n");
        exit(-1);
    }else{
        if(atoi(argv[1])==0){
            wait(&status);
        }else{
            alarm(atoi(argv[1]));
            fprintf(stderr, "%s is timed out\n", "child");
            kill(pid,SIGKILL);
        }
    }

    return 0;
}
*/

/* pipe() practice

int main(){
    int pipefd[2];
    pid_t cpid;
    char buf;

    char talktochild[100] = "성민이의 2019년 운체 pipe 실습 ^^";
    char* pointer = talktochild;

    if(pipe(pipefd) == -1 ){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid= fork();
    if(cpid == 0){
        close(pipefd[1]);
        while(read(pipefd[0],&buf,1) > 0) // 반복문을 써서 문자하나하나 씩 read했다. 그래서 char buf 인데도 문자열 출력 가능했던거임
        write(STDOUT_FILENO,&buf,1);

        write(STDOUT_FILENO,"\n",1 );
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }else{
       close(pipefd[0]);

        write(pipefd[1],pointer,strlen(pointer));
        close(pipefd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
*/