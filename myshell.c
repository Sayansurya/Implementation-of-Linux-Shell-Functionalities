#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
int fc3, fc2, foreground, k = 0;
int arr_c1[64];
/* Splits the string by space and returns the array of tokens
 *
 */
void handler_func(int sigusr2)
{
	kill(fc3, SIGKILL);
	waitpid(fc3, NULL, 0);
	exit(0);
}
void INThandler(int sigint)
{
	// printf("ctrl+c caught\n");
	printf("\n");
	if (foreground != 0)
		kill(foreground, SIGKILL);
}

void reapChild(int sigusr1)
{
	printf("Shell: Background process finished\n"); // printing but after that no $ is coming.
	sleep(1);
	int pid = waitpid(-1, NULL, WNOHANG);
	while (pid > 0)
	{
		pid = waitpid(-1, NULL, WNOHANG);
	}
	// printf("Shell: Background process finished\n");
}

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, reapChild);
	signal(SIGUSR2, handler_func);
	signal(SIGINT, INThandler);

	char line[MAX_INPUT_SIZE];
	char path[MAX_INPUT_SIZE];
	char **tokens;
	int i;

	while (1)
	{
		/* BEGIN: TAKING INPUT */
		int wstatus;

		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();
		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);
		// do whatever you want with the commands, here we just print them
		if (tokens[0] == NULL)
		{
			continue;
		}
		int bp = 0;
		// ctrl + c handling
		signal(SIGINT, INThandler);
		if (strcmp(tokens[0], "^c") == 0)
		{
			// printf("ctrl+c recieved\n");
			for (int iter = 0; iter < 64; iter++)
			{
				if (arr_c1[iter] == 0)
					break;
				signal(SIGINT, INThandler);
				kill(arr_c1[iter], SIGUSR1);
				exit(0);
			}
		}

		if (strcmp(tokens[0], "exit") == 0)
		{
			// printf("Kill Command Received\n");

			for (int iter = 0; iter < 64; iter++)
			{
				if (arr_c1[iter] == 0)
					break;
				// printf("Killing: %d\n", arr_c1[iter]);
				kill(arr_c1[iter], SIGUSR2);
			}
			while (wait(NULL) > 0)
				;
			// printf("Terminating Shell.....");
			exit(0);
		}

		int i = 0;
		while (tokens[i] != NULL)
		{
			i++;
		}

		if (tokens[i - 1][0] == '&')
		{
			// background process
			// printf("Background processs building \n");
			tokens[i - 1] = NULL;

			i = 0;

			fc2 = fork(); // c1 process
			arr_c1[k] = fc2;
			k++;
			// printf("%d\n", arr_c1[k - 1]);

			if (fc2 < 0)
			{
				fprintf(stderr, "%s\n", "unable to create child process \n");
			}
			else if (fc2 == 0)
			{
				setpgid(0, getpid());
				// printf("Another child\n");
				fc3 = fork(); // c2 process
				if (fc3 < 0)
				{
					fprintf(stderr, "%s\n", "unable to create child process \n");
				}
				else if (fc3 == 0)
				{
					execvp(tokens[0], tokens);
					printf("Command execution failed\n");
					exit(1);
				}
				else
				{
					int wc = waitpid(fc3, NULL, 0);
					kill(getppid(), SIGUSR1);
					exit(0);
				}
				// printf("Hello");
				// printf("%d", getpid());
			}
		}
		else
		{
			// printf("Foreground\n");

			// directory change

			if (strcmp(tokens[0], "cd") == 0)
			{
				if (tokens[2] != NULL)
				{
					printf("Too many arguments\n");
				}
				else
				{
					i = 1;
					int j = 0;
					char dir[1024];
					while (tokens[i][j] != '\0')
					{
						dir[j] = tokens[i][j];
						j++;
					}

					dir[j] = '\0';
					// printf("%s \n", dir);

					if (chdir(dir) != 0)
					{
						perror("Error");
					}
				}
			}

			else
			{
				foreground = fork();
				if (foreground < 0)
				{
					fprintf(stderr, "%s\n", "unable to create child process \n");
				}
				else if (foreground == 0)
				{
					execvp(tokens[0], tokens);
					printf("Command execution failed\n");
					exit(0);
				}
				else
				{
					waitpid(foreground, NULL, 0);
					foreground = 0;
				}
			}
		}

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		free(tokens);
	}

	return 0;
}
