#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
pid_t new_pid, child_pid;

void main()
{
	int stat_val;
	char *words[100];
	char proc[100];
	char sep[] = " ";
	while(1) {
		printf("Please, enter the name of the process:\n");
		scanf("%s",proc);
		int i = 0;
		char *ptr = strtok(proc, sep);
		while (ptr != NULL) {
			words[i++] = ptr;
			ptr = strtok (NULL, sep);
		}
		if (i > 1 && strcmp(words[i-1], "x") == 0) {
			words[i-1] = NULL;
		}
		else words[i] = NULL;
		if (strcmp(words[0], "break") == 0)
			exit(EXIT_SUCCESS);
		if (strcmp(words[0], "cd") == 0) {
			chdir(words[1]);
			continue;
		}

		new_pid=fork();
		switch(new_pid) {
		case -1:
			perror("fork failed"); 
			exit(EXIT_FAILURE);
		case 0:			
			printf(" CHILD: This is the child process!\n");
			printf(" CHILD: My PID -- %d\n", getpid());
			printf(" CHILD: My parent PPID -- %d\n", getppid());
			printf(" CHILD: My process was changed\n");
			(void) signal(SIGINT,SIG_DFL);
			execvp(words[0],words);
			kill(getppid(), SIGALRM);

		default:                
			printf("PARENT: This is the parent process!\n");
			printf("PARENT: My PID -- %d\n", getpid());
			printf("PARENT: My child PID %d\n",new_pid);
			printf("PARENT: I'm waiting for the completion of the child process.\n");
			if (new_pid) {

				child_pid = wait(&stat_val);
				printf("Child has finished: PID = %d\n",child_pid);
				if (WIFEXITED(stat_val))
					printf("Child exited with code %d\n",WIFEXITED(stat_val));
				else
					printf("Child terminated abnormally\n");
			}
		}
	}
}

