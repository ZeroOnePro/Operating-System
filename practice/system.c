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

int main(int argc, char* argv[]){
    
    int status;
    pid_t pid;

    if((pid=fork())==0){
        if(execvp(argv[0],argv)<0){
            fprintf(stderr,"No such file of directory\n");
            abort();
        }
    }else{
        waitpid(pid,&status,0);
    }

    fprintf(stderr,"부모새끼\n");

    return 0;
}