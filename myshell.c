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


int parseInput(char *buffer,char *commands[],int *nextNull,int *size,int *childCounter)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	printf("inside parseInput\n");
	char *command;
	// int size=0;
	int functionIndex=0;
	while((command = strsep(&buffer," ")) != NULL )
    {
		if (strcmp(command,"&&")==0)
		{
			functionIndex=1;
			commands[*size]=NULL;
			nextNull[*childCounter-1]=*size;
			*size+=1;

			*childCounter+=1;
		}
		else if (strcmp(command,"##")==0)
		{
			functionIndex=2;
			commands[*size]=NULL;
			nextNull[*childCounter-1]=*size;
			*size+=1;
			*childCounter+=1;

			
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
			// nextNull[*size]=21;
        	*size+=1;
		}

    }
	commands[*size]=NULL;

	return functionIndex;

	
	
}

void changeDirectory(char *str)
{
	printf("change directory function\n");
	printf("path %s\n",str);
	printf("status %d\n",chdir(str));
	if(str!=NULL)
    {
        if(chdir(str)==-1)
        {   
            printf("Shell: Incorrect command\n");
            
        }  
    }
}

void executeCommand(char *commands[],int  marker)
{
	// This function will fork a new process to execute a command
	printf("executeCommand function\n");
	printf("%s %s\n",commands[0],commands[1]);
	if (strcmp(commands[marker],"cd")==0)
	{
		printf("calling change directory\n");
		changeDirectory(commands[marker+1]);
	}
	else
	{
		int child = fork();     
        if (child < 0){	
            // fork failed; exit
            exit(0);
        }
        else if (child == 0) {
            signal(SIGINT, SIG_DFL);	
            signal(SIGTSTP, SIG_DFL);	   // child (new) process
            if(execvp(commands[marker], commands+marker)<0)
            {
                printf("Shell: Incorrect command\n");
            }
        } 
        else {              // parent process (rc holds child PID)
            int rwait = wait(NULL);
        }
	}
	
}

void executeParallelCommands(char *commands[],int *nextNull,int childNum)
{
	// This function will run multiple commands in parallel
	int *childs = (int *)malloc(sizeof(int)*(childNum));
	int marker=0,update=0;
	childs[0]=fork();
	if (childs[0] < 0){		     	// fork failed; exit
		exit(0);
	}
	else if (childs[0] == 0) {	
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);		
        if(strcmp(commands[0],"cd")==0)
        {
			changeDirectory(commands[1]);
        }
        else
        {
            if(execvp(commands[0], commands)<0)
            {
                 printf("Shell: Incorrect command\n");
            }
        }
        
		
	} 
	else {         
		     // parent process (rc holds child PID)
		printf("2nd child activated\n");
		for (int i = 1; i < childNum; i++)
		{
			if (update<childNum)
			{
				marker = nextNull[update]+1;
				
			}
			
			childs[i] = fork();
			if (childs[i] < 0){			// fork failed; exit
				exit(0);
			}
			else if (childs[i] == 0) {		// child (new) process 2
        	    signal(SIGINT, SIG_DFL);
        	    signal(SIGTSTP, SIG_DFL);		
        	    if(strcmp(commands[marker],"cd")==0)
        	    {
					changeDirectory(commands[marker+1]);
        	        
        	    }
        	    else
        	    {
        	        if(execvp(commands[marker], commands+marker)<0)
        	        {
        	            printf("Shell: Incorrect command\n");
        	        }
        	    }

			}
			else {              // parent process (rc holds child PID)
				int rc_wait2 = wait(NULL);
			}
			update++;

		}
		
	}
	



}

void executeSequentialCommands(char *commands[],int *nextNull,int childNum)
{	

	int marker=0,update=0;
	for (int i = 0; i < childNum; i++)
	{
		executeCommand(commands,marker);
		if (update<childNum)
		{
			marker=nextNull[update]+1;
		}
		update++;
		
	}
	
	// This function will run multiple commands in sequence
	// int *childs = (int *)malloc(sizeof(int)*(childNum));
	// int marker=0,update=0;
	// int *rc_wait = (int *)malloc(sizeof(int)*childNum);
	// childs[0]=fork();
	// if (childs[0] < 0){		     	// fork failed; exit
	// 	exit(0);
	// }
	// else if (childs[0] == 0) {	
    //     signal(SIGINT, SIG_DFL);
    //     signal(SIGTSTP, SIG_DFL);		
    //     if(strcmp(commands[0],"cd")==0)
    //     {
	// 		changeDirectory(commands[1]);
    //     }
    //     else
    //     {
    //         if(execvp(commands[0], commands)<0)
    //         {
    //              printf("Shell: Incorrect command\n");
    //         }
    //     }
        
		
	// } 
	// else {         
	// 	     // parent process (rc holds child PID)
	// 	printf("2nd child activated\n");
	// 	for (int i = 1; i < childNum; i++)
	// 	{
    //     	int rc_wait[i] = wait(NULL);             // parent process (rc holds child PID)

	// 		if (update<childNum)
	// 		{
	// 			marker = nextNull[update]+1;
				
	// 		}

			
	// 		childs[i] = fork();
	// 		if (childs[i] < 0){			// fork failed; exit
	// 			exit(0);
	// 		}
	// 		else if (childs[i] == 0) {		// child (new) process 2
    //     	    signal(SIGINT, SIG_DFL);
    //     	    signal(SIGTSTP, SIG_DFL);		
    //     	    if(strcmp(commands[marker],"cd")==0)
    //     	    {
	// 				changeDirectory(commands[marker+1]);
        	        
    //     	    }
    //     	    else
    //     	    {
    //     	        if(execvp(commands[marker], commands+marker)<0)
    //     	        {
    //     	            printf("Shell: Incorrect command\n");
    //     	        }
    //     	    }

	// 		}
	// 		else {              // parent process (rc holds child PID)
	// 			int rc_wait[i+1] = wait(NULL);
	// 		}
	// 		update++;

	// 	}
		
	// }
}

void executeCommandRedirection()
{
	// This function will run a single command with output redirected to an output file specificed by user
}

int main()
{
	// Initial declarations
	char *buffer;
	char *directory;
	char *exitStatus = "exit";
	int runProg=1;
	

    size_t bufsize = 300;
    size_t characters;
	
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	
	while(runProg)	// This loop will keep your shell running until user exits.
	{
		char *commands[20];
		int *nextNull;
		nextNull = (int *)malloc(sizeof(int)*20);
		int functionIndex,size=0,childCounter=1;
		// Print the prompt in format - currentWorkingDirectory$
		printf("%s$", getcwd(directory, 500));
		
		// accept input with 'getline()'
		buffer =  (char *)malloc(sizeof(char)*bufsize);
		characters = getline(&buffer,&bufsize,stdin);
		buffer[characters-1]='\0';
		printf("input taken\n");
		
		if(strcmp(buffer,exitStatus)==0)	// When user uses exit command.
		{
			printf("Exiting shell...");
			runProg=0;
		}
		else
		{
			printf("inside else\n");

			//  puts ("Please enter a line of text.");

  			/* These 2 lines are the heart of the program. */
  			//my_string = (char *) malloc (nbytes + 1);
  			//command = getline (&my_string, &nbytes, stdin);
			// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
			functionIndex=parseInput(buffer,commands,nextNull,&size,&childCounter); 	

			printf("after funtionIndex\n");

			printf("functionIndex %d\n",functionIndex);

			for (int i = 0; i < size; i++)
			{
				printf("%s ",commands[i]);
			}

			for (int i = 0; i < childCounter; i++)
			{
				printf("%d ",nextNull[i]);
			}			


			if(functionIndex==1)
				executeParallelCommands(commands,nextNull,childCounter);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			else if(functionIndex==2)
				executeSequentialCommands(commands,nextNull,childCounter);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			else if(functionIndex==3)
				executeCommandRedirection();	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
			else
				executeCommand(commands,0);		// This function is invoked when user wants to run a single commands
		
		}
		
	}
	
	return 0;
}
