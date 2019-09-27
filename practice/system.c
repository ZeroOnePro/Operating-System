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

int main(int argc, const char* argv[]){
    int status;
    struct sigaction act;
    act.sa_handler = set_timeout;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM,&act,0);
    alarm(2);
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
            wait(&status);
            fprintf(stderr, "%s is timed out\n", "child");
            kill(pid,SIGKILL);
        }
    }

    return 0;
}
