#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include "version.h"
char line[1024];
int n = 0;
char* args[1024];
pid_t pid;
int command_pipe[10];

void clearUp(int n)
{
	int i;
	for (i = 0; i < n; ++i) 
		wait(NULL); 
}

int command(int input, int begin, int end)
{
	int x[2];
	pipe(x);	
	pid = fork();
	if (pid == 0) 
	{
		if (begin == 1 && end == 0 && input == 0) 
		{
			dup2(x[1], STDOUT_FILENO);
		} 
		else if (begin == 0 && end == 0 && input != 0) 
		{
			dup2(input, STDIN_FILENO);
			dup2(x[1], STDOUT_FILENO);
		} 
		else 
		{
			dup2(input, STDIN_FILENO);
		}
		if (execvp(args[0], args) == -1)
		{
			_exit(EXIT_FAILURE);
		}
	}
	if (end == 1)
	{
		close(x[0]);
	}
	if (input != 0) 
	{
		close(input);
	}
	close(x[1]);
	return x[0];
}
 
int run(char* cmd, int input, int begin, int end);
void split(char* cmd);
 
int run(char* cmd, int input, int begin, int end)
{
	split(cmd);
	if (args[0] != NULL) 
	{
		if (strcmp(args[0], "exit") == 0) 
		{
			exit(0);
		}
		n += 1;
		return command(input, begin, end);
	}
	return 0;
}

char* trimToSize(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}
 
void split(char* cmd)
{
	cmd = trimToSize(cmd);
	char* next = strchr(cmd, ' ');
	int i = 0;
	while(next != NULL) 
	{
		next[0] = '\0';
		args[i] = cmd;
		++i;
		cmd = trimToSize(next + 1);
		next = strchr(cmd, ' ');
	}
	if (cmd[0] != '\0') {
		args[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		++i; 
	}
	args[i] = NULL;
}

int main()
{
	displayVersion();
	printf("Sorairono's Custom Shell (Type 'exit' to exit the shell)\n");
	while (1) 
	{
		printf("Sorairono's Shell>$ ");
		fflush(NULL);
		if (!fgets(line, 1024, stdin)) 
		{
			return 0;
		}
		int begin = 1;
		int input = 0;
		char* cmd = line;
		char* command = strchr(cmd, '|');
		while (command != NULL) 
		{
			*command = '\0';
			input = run(cmd, input, begin, 0);
			cmd = command + 1;
			command = strchr(cmd, '|'); 
			begin = 0;
		}
		input = run(cmd, input, begin, 1);
		clearUp(n);
		n = 0;
	}
}