/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the signal handler code (which should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you dp not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()


int parseInput(char *input,char *commands[],int *nextNull,int *size,int *childCounter)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	
	char *command;
	int functionIndex=0;
	while((command = strsep(&input," ")) != NULL )
    {	
		/* whenever a command taken from input during getline finds a delimiter operator 
		 it adds NULL value into the commands string array */
		/*Whenever the delimeter operator is detected childCounter is incremented so as to keep track of childs require for parallel execution*/
		// nextNull array keeps track of position where we have added NULL value into the commands[] string array
		if (strcmp(command,"&&")==0)
		{
			commands[*size]=NULL;
			nextNull[*childCounter-1]=*size;
			*size+=1;
			functionIndex=1;
			*childCounter+=1;
		}
		else if (strcmp(command,"##")==0)
		{
			commands[*size]=NULL;
			nextNull[*childCounter-1]=*size;
			*size+=1;
			*childCounter+=1;
			functionIndex=2;
			
		}
		else if (strcmp(command,">")==0)
		{
			functionIndex=3;
			commands[*size]=NULL;
			nextNull[*childCounter-1]=*size;
			*size+=1;
			*childCounter+=1;
		}
		else
		{
			commands[*size]=command;
        	*size+=1;
		}

    }
	commands[*size]=NULL;//after all the commands added it adds Null in the end
	return functionIndex;
	// functionIndex returns an integer to specify which type of operation to perform in main function
}

void changeDirectory(char *pathFolder)
{
	// function to change current directory to a specified directory 
	if(pathFolder!=NULL)//enters only if string is not empty
    {
        if(chdir(pathFolder)==-1)//checks  if the directory exists and whether it is possible to change
        {   
            printf("Shell: Incorrect command\n");
        }  
    }
}

void executeCommand(char *commands[],int  marker)
{
	// executecommand function accepts commands[] string array and marker(which is the position of command to perform)
	if (strcmp(commands[marker],"cd")==0)//checks for cd command
	{
		changeDirectory(commands[marker+1]);
	}
	else
	{
		int child = fork(); //new child
        if (child < 0)
		{	
            exit(0); // fork failed; exit
        }
        else if (child == 0) 
		{
            signal(SIGINT, SIG_DFL);	
            signal(SIGTSTP, SIG_DFL);	   // child (new) process
            if(execvp(commands[marker], commands+marker)<0)
            {
                printf("Shell: Incorrect command\n");//Incorrect command
            }
			exit(0);//exit the process
        } 
        else
		{    // parent process (childWait holds child PID)
            int childWait = wait(NULL);
        }
	}	
}

void executeParallelCommands(char *commands[],int *nextNull,int childNum)
{
	// This function will run multiple commands in parallel
	/* the function accepts command[] string array , nextNull integer array which specifies the position of null in commands[] array
	 and childNum which is the number of childs to be created*/

	int *childs = (int *)malloc(sizeof(int)*(childNum));
	int marker=0,update=0;
	childs[0]=fork();
	if (childs[0] < 0)
	{		     	
		exit(0);// fork failed; exit
	}
	else if (childs[0] == 0) 
	{	
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);		
        if(strcmp(commands[0],"cd")==0)//checks for cd command
        {
			changeDirectory(commands[1]);
        }
        else
        {
            if(execvp(commands[0], commands)<0)
            {
                 printf("Shell: Incorrect command\n");
            }
			exit(0);
        }
	} 
	else {         
		  
		for (int i = 1; i < childNum; i++)
		{
			if (update<childNum)
			{
				marker = nextNull[update]+1;// change the marker(which points to position of next command) with track of nextNull array position
			}
			
			childs[i] = fork();
			if (childs[i] < 0)
			{			
				exit(0);// fork failed; exit
			}
			else if (childs[i] == 0) 
			{	
				// child (new) process 
        	    signal(SIGINT, SIG_DFL);
        	    signal(SIGTSTP, SIG_DFL);		
        	    if(strcmp(commands[marker],"cd")==0)//checks for cd command
        	    {
					changeDirectory(commands[marker+1]);
        	    }
        	    else
        	    {
        	        if(execvp(commands[marker], commands+marker)<0)
        	        {
        	            printf("Shell: Incorrect command\n");
        	        }
					exit(0);
        	    }
			}
			else 
			{
				int childWait = wait(NULL);
			}
			update++;
		}
	}
}

void executeSequentialCommands(char *commands[],int *nextNull,int childNum)
{	
	// This functions executes commands in a sequence manner with FIFO
	/* the function accepts command[] string array , nextNull integer array which specifies the position of null in commands[] array
	 and childNum to track how many process needs to be executed*/

	int marker=0,update=0,i=0;
	while (i < childNum)
	{
		// calling executeCommand everytime to execute commands in commands[] array sequentially
		executeCommand(commands,marker);
		if (update<childNum)
		{
			marker=nextNull[update]+1;
			// change the marker(which points to position of next command) with track of nextNull array position
		}
		update++;
		i++;
	}
}

void executeCommandRedirection(char *filename,char *commands[])
{
	// This function will run a single command with output redirected to an output file specificed by user
	int child = fork();
	if (child < 0)
	{
		exit(0);// fork failed; exit
	}
	else if (child == 0) 
	{
		close(STDOUT_FILENO);
		open(filename,O_CREAT | O_WRONLY | O_APPEND);
		if (execvp(commands[0], commands)<0)
		{
            printf("Shell: Incorrect command\n");
		}
		exit(0);
	} 
	else 
	{
		int childWait = wait(NULL);// parent process (childWait holds child PID)
	}
}

int main()
{
	// Initial declarations
	char *inputBuffer,*directory,*exitStatus = "exit";
	int runProg=1;
    size_t inputLength = 300,characters;

	while(runProg)	// This loop will keep your shell running until user exits.
	{	
		signal(SIGINT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		int *nextNull,functionIndex,size=0,childCounter=1;
		nextNull = (int *)malloc(sizeof(int)*20);
		char *commands[20];
		// Print the prompt in format - currentWorkingDirectory$
		printf("%s$", getcwd(directory, 500));
		// accept input with 'getline()'
		inputBuffer =  (char *)malloc(sizeof(char)*inputLength);
		characters = getline(&inputBuffer,&inputLength,stdin);
		inputBuffer[characters-1]='\0';
	
		
		if(strcmp(inputBuffer,exitStatus)==0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			runProg=0;//setting to 0 to exit the while loop
		}
		else
		{
			// calling parseInput function to break the input into separate words(command)
			functionIndex=parseInput(inputBuffer,commands,nextNull,&size,&childCounter); 	
			// functionIndex will get an integer to specify which operation to perform
			if(functionIndex==1)
			{
				executeParallelCommands(commands,nextNull,childCounter);// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			}
			else if(functionIndex==2)
			{
				executeSequentialCommands(commands,nextNull,childCounter);// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			}
			else if(functionIndex==3)
			{
				int marker = nextNull[0]+1;
				char *filename=commands[marker];// filename will have the value specified by user after ">" delimitery operator
				executeCommandRedirection(filename,commands);// This function is invoked when user wants redirect output of a single command to and output file specificed by user
			}	
			else
			{
				executeCommand(commands,0);// This function is invoked when user wants to run a single commands
			}		
		}		
	}
}
