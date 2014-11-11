#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int error;
char buffer[1024];
pid_t pid;
int killed = 0;

void handler(int signum)
{
	write(1,"You are the weakest link, goodbye \n", 36);
	killed = 1;
	kill(pid, SIGKILL);
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		write(1,"This program requires 2 args \n", 30); 
		return -1; //Exit without segfault. 
	}
	int timer = atoi(argv[1]);
	signal(SIGALRM,handler);
	while(1)
	{ 
		killed = 0;
		write(1,"WeakLink# ", 10);
		error = read(0, buffer, 1024);
		if(error < 0)
			perror("da fuq");
		
		int i = 0;
		while(buffer[i] != '\n' && buffer[i] != ' ' && buffer[i] != '\0' && buffer[i] != EOF)
		{
			i++;
		}
		alarm(timer);
		
		buffer[i] = '\0';
		
		pid = fork();
		
		if(!pid) // child process... if pid is 0
		{	
			char *argz[] = {buffer, NULL};
			error = execve(buffer, argz, NULL);
			if(error < 0)
				perror("Execution Error: ");
			exit(100);
		}
		else //parent process waits for child...
		{
			wait();
			alarm(0); //reset alarm clock
			if(killed == 0)
			{
				write(1, "Your link is not the weakest!\n", 30);
				alarm(0);
			}
		}
	}
}
