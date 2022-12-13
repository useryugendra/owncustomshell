#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit(), function call mentioned in the program is present in this library
#include <unistd.h>			// fork(), getpid(), exec(), function calls mentioned in the program are present in this library
#include <sys/wait.h>		// wait(), function call mentioned in the program is present in this library
#include <signal.h>			// signal(), function call mentioned in the program is present in this library
#include <fcntl.h>			// close(), open(), function call mentioned in the program is present in this library



/* in the parseInput function 
the input is separated whenever a space occurs and create a it like a 2d array of charecters and 
the last pointer is made null so,
we can identify that we have reached to the end of the input*/
char** parseInput(char* input)
{
	char **arr = malloc(128*sizeof(char*));
	if(!arr)
	{
		printf("Allocation error\n");
		exit(EXIT_FAILURE);
	}
	char* a;
	int position = 0;
	while((a = strsep(&input," ")) != NULL)
	{
		arr[position] = a;
		position++;
	}
	arr[position] = NULL;
	return arr;
}

/*in executeCommand function 
we if we have cd as the given command,
we will call chdir() which will change the directory to the directory mentioned in the argument
else we will fork() the process and create a child process where the commands run in the child by the function call execvp()
function call and the parent waits until the child completes execution
*/
void executeCommand(char** argument)
{
	if(argument[0] == NULL)
		return;
	else if(strcmp(argument[0],"cd") == 0)
	{
		chdir(argument[1]); // chdir changes directory to the directory mentioned in the argument
		return;
	}
	else
	{
		int rc = fork();
  		if (rc == 0) // If the fork creates a child process
  		{
    		if (execvp(argument[0], argument) < 0)
    		{
      			printf("Shell: Incorrect command\n");
				exit(1);
    		}
  		}
  		else if (rc > 0) // for parent process
  		{
			int r = wait(NULL);
  		}
  		else  // If the forking fails
  		{
    		perror("error");
    	}
    	return;
	}
}

/*
in executeParallelCommands() 
there are 2 while loops
where the 1st one runs till the end of our 2d array of charecters
and the second one runs till it encounters '&&' for each row in the 2d array of charecters
and runs the commands before and after of '&&' parallely
*/
void executeParallelCommands(char ** argument)
{
	int rc;
	int i = 0;
	int j = i;
	while(argument[i] != NULL)
	{
		while(argument[i] != NULL && strcmp(argument[i],"&&") != 0)
		{
			i++;
		}
		argument[i] = NULL;
		rc = fork();
		if(rc == 0) // If the fork creates a child process
		{
			if(execvp(argument[j],&argument[j]) < 0)
			{
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		}
		if(rc < 0) // If error in forking
		{
			exit(1);
		}
		i++;
		j = i;
	}
	int rc_wait=wait(NULL); //in the wait function call parent process waits for child processes to complete
}

/*
in executeSequentialCommands() 
there are 2 while loops
where the 1st one runs till the end of our 2d array of charecters
and the second one runs till it encounters '##' for each row in the 2d array of charecters32
and runs the commands before and after of '##' sequentially
*/

void executeSequentialCommands(char** argument)
{
	int i = 0;
	int j = i;
	while(argument[i] != NULL)
	{
		while(argument[i]!= NULL && strcmp(argument[i],"##") != 0)
		{
			i++;
		}
		argument[i] = NULL;
		executeCommand(&argument[j]);// after this execution is completed, another command starts running
		i++;
		j = i;
	}

}

void executeCommandRedirection(char** argument)
{
	int i = 0;
	while(argument[i] != NULL && strcmp(argument[i],">") != 0)
	{
		i++;
	}
	argument[i] = NULL;
	int rc = fork();
	if(rc == 0)		// If the fork creates a child process
	{
		close(STDOUT_FILENO);
		open(argument[i+1], O_CREAT | O_WRONLY | O_APPEND);
		if(execvp(argument[0],argument)<0) // in case of wrong command
		{
			printf("Shell: Incorrect command");
			exit(1);
		}
	}
	else if(rc > 0)		// for parent process
	{
		int r = wait(NULL);  //in the wait function call parent process waits for child processes to complete
	}
	else        // error in forking
	{
		exit(1);
	}

}



int main()
{
	long unsigned int size = 10;
	char* input;
	input = (char *)malloc(size);
	if(!input)
	{
		printf("Allocation error\n");
		exit(EXIT_FAILURE);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	while(1)	// this is a infinite loop which only exits when we gives exit as input
	{
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		printf("%s$",cwd);
		// printing the PATH

		int i = 0;
		getline(&input,&size,stdin);
		while(input[i] != '\0')
		{
			if(input[i] == EOF || input[i] == '\n')
			{
				input[i] = '\0';
			}
			i++;
		}
		char ** arr;
		arr = parseInput(input);
		if(strcmp(arr[0],"exit") == 0)	// here when exit is passed as input the loop exits
		{
			printf("Exiting shell...\n");
			break;
		}
		i = 0;
		int select = 0;
		while(arr[i] != NULL)
		{
			if(strcmp(arr[i],"&&") == 0)
			{
				select = 1;
				break;
			}
			else if(strcmp(arr[i],"##") == 0)
			{
				select = 2;
				break;
			}
			else if(strcmp(arr[i],">") == 0)
			{
				select = 3;
				break;
			}
			i++;
		}
		if(select == 1)
		{
			executeParallelCommands(arr);
		}	// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(select == 2)
		{
			executeSequentialCommands(arr);
		}	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(select == 3)
		{
			executeCommandRedirection(arr);
		}	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
		{
			executeCommand(arr);
		}	// This function is invoked when user wants to run a single commands

	}
	return 0;
}

