#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char* argv[])
{
    char buffer[1024];
			getcwd(buffer,1024);
			printf("%s\n",buffer);

    return 0;
}