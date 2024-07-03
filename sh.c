/**************************
	Simple UNIX Shell
  	By: Abdul-Hakim Zekri
***************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_INPUT_SIZE 255 // Maximum input string length
#define MAX_ARGS  255 // Maximum arguments
#define MAX_PATHS 64 // Maximum number of paths
#define MAX_CMDS 64 // Maximum number of commands

char *path[MAX_PATHS];
int num_paths = 1; // Number of paths stored

void shellerror()
{
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message)); 
	fflush(stderr);
}

int create_process(char *path, char *args[], char *output_file)
{
	// Initiate a child process
	int pid = fork();

	if (pid < 0) 
		return -1; // Fork failed
	else if (pid == 0) 
	{
		if(output_file != NULL)
		{
			close(STDOUT_FILENO);
			open(output_file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
		}
		// Child process
		if (execv(path, args) == -1) 
			return -1; // execv failed
	} 

	return pid;
}

int main(int argc, char *argv[]) 
{
	if (argc > 1) {
		// If any arguments besides sh are provided, exit with status 1
		shellerror();
		exit(1);
	}
	
	char *input = NULL;
	size_t bufferSize = 0;
	ssize_t charsRead;

	path[0] = strdup("/bin"); // Add /bin as first directory

	// Infinite loop until the user inputs the 'exit' command
	while (1) 
	{
		printf("|> ");
		fflush(stdout); // Ensure the prompt is displayed immediately
		
		charsRead = getline(&input, &bufferSize, stdin); // Read a line of input
		if (charsRead == -1) 
		{
			free(input);
			shellerror();
			continue; // In case of an error, prompt again
		}

		if(input != NULL)
		{
			// Check if the line is empty or consists only of whitespace characters
			size_t len = strspn(input, " \t\n");

			// If the length of the initial segment containing only whitespace is equal to the length of the string,
			// then the entire line is empty or whitespace.
			if(input[len] == '\0')
				continue;    // Skip the rest of the loop iteration
		}

		// Remove newline character from input
		if (input[charsRead - 1] == '\n') 
			input[charsRead - 1] = '\0';

		// Parse and execute the command
		char *cmd = NULL;
		char *args[MAX_INPUT_SIZE / 2 + 1]; // Argument array
		int i = 0;

		// Use strsep to parse the input into arguments
		while ((cmd = strsep(&input, " \t")) != NULL) 
		{
			if (*cmd == '\0') continue; // Skip any empty strings
			args[i++] = cmd;
		}
		args[i] = NULL; // Null-terminate the argument array

/*------HERE ALL ARGUMENTS HAVE BEEN SEPARATED BY WHITESPACE CHARACTERS---------*/		

		int cmd_error = 0;

		// Check for built-in commands
		if (strcmp(args[0], "exit") == 0) 
		{
			if (args[1] != NULL) 
			{
				shellerror();
				continue;
			} 
			else 
				exit(0);
		} 
		else if (strcmp(args[0], "cd") == 0) 
		{
			if (i != 2) 
			{
				shellerror();
				continue;
			} 
			else 
			{
				if (chdir(args[1]) != 0) 
				{
					shellerror();
					continue;
				}
			}
		} 
		else if (strcmp(args[0], "path") == 0) 
		{
			int num_args = i;
			// Clear out current paths
			for (int j = 0; j < num_paths; j++) 
			{
				path[j] = NULL; 
			}
			num_paths = 0;

			// Tterate over the arguments and add them to the path array
			for (int j = 1; j < num_args && num_paths < MAX_PATHS; j++)
			{
				path[num_paths] = strdup(args[j]); // Duplicate and store the path
				num_paths++; // Increment the count of paths stored
			}			
		}
		else if(path[0] != NULL)//Handle process creation
		{
			int parallel_index[MAX_CMDS] = {0}; // Indices of "&" in args
			char *commands[MAX_CMDS][MAX_ARGS]; // Array to hold separated commands

			int cmd_count = 0; // Number of commands found
			int start_index = 0; // Start index for the current command in args

			// Populate parallel_index with indices of "&"
			for (int j = 0; j < i; j++) 
			{
				if (strcmp(args[j], "&") == 0) 
				{
					parallel_index[cmd_count++] = j;
				}
			}

			// Separate args into commands based on parallel_index
			for (int i = 0; i <= cmd_count; i++) 
			{
				int end_index;
				// Use parallel_index or end of args
				if (i < cmd_count) 
					end_index = parallel_index[i];
				else 
					end_index = MAX_ARGS;

				int arg_index = 0; // Index for the current command's arguments

				// Copy arguments for the current command
				for (int j = start_index; j < end_index && args[j] != NULL; j++) 
				{
					commands[i][arg_index++] = args[j];
				}
				commands[i][arg_index] = NULL; // Null-terminate the current command's arguments

				start_index = end_index + 1; // Update start_index for the next command
			}

			for (int i = 0; i <= cmd_count; ) {
				if (commands[i][0] == NULL) {  // Check if the first argument of the command is NULL
					// Shift all subsequent commands one position up
					for (int j = i; j < cmd_count - 1; j++) {
						int k;
						// Move each argument individually
						for (k = 0; commands[j + 1][k] != NULL; k++) {
							commands[j][k] = commands[j + 1][k];
						}
						// Null-terminate the shifted command
						commands[j][k] = NULL; // 'k' is the "appropriate index" here
					}
					// Null-terminate the last command after shifting
					commands[cmd_count - 1][0] = NULL;
					cmd_count--;  // Decrement the command count
				} else {
					i++;  // Move to the next command only if the current command is not NULL
				}
			}

			
/*---------- HERE WE HAVE SEPARATED COMMANDS BASED ON THE & SYMBOL ---------------------*/


			int pids[MAX_CMDS]; // Array to store child PIDs
			int num_processes = 0; // To keep track of the number of processes started

			for (int i = 0; i <= cmd_count && commands[i] != NULL; i++)
			{
				char executable[MAX_ARGS]; // Ensure this is large enough to hold the full path
				int redirect_index = -1; // To store the index of the ">" in args
				char *output_file = NULL; // To store the output redirection file, if any
				for(int j = 0; j < num_paths; j++)
				{
					// Construct the full path to the executable
					char full_path[256]; // Ensure this is large enough to hold the full path
					strcpy(full_path, path[j]);
					strcat(full_path, "/");
					strcat(full_path, commands[i][0]);
					// Check if the file exists and is executable
					if(access(full_path, X_OK) == 0)
					{
						strcpy(executable, full_path);
						break; // Exit the loop if the executable is found
					}
					else if(j == num_paths - 1) // If it's the last iteration and the file is still not found
					{
						cmd_error = -1;
						break;
					}
				}
				for(int k, redirect_num = 0; commands[i][k] != NULL; k++) 
				{
					if(strcmp(commands[i][k], ">") == 0) 
					{
						redirect_num++;
						if(commands[i][k+1] != NULL && commands[i][k+2] == NULL) // Ensure correct format
						{ 
							output_file = commands[i][k+1];
							redirect_index = k;
							commands[i][k] = NULL; // Terminate args array before ">"
							break;
						} 
						else 
						{
							cmd_error = -1; // Handle error for incorrect redirection format
							break;
						}
					}
					if(redirect_num > 1)
					{
						cmd_error = -1;
						break;
					}
				}

				if(cmd_error < 0)
					break;

				// Now we handle process creation
				if(redirect_index == -1)
				{
					// Without redirection
					pids[num_processes] = create_process(executable, commands[i], NULL);
					if( pids[num_processes] == -1)
					{
						cmd_error = -1;
						break;
					}
					else {num_processes++;}
				}
				else 
				{
					// With redirection
					pids[num_processes] = create_process(executable, commands[i], output_file);
					if( pids[num_processes] == -1)
					{
						cmd_error = -1;
						break;
					}	
					else {num_processes++;}
				}
			}

			if(cmd_error < 0)
			{
				shellerror();
				continue;
			}

			// Wait for all processes to finish
			for (int i = 0; i < num_processes; i++)
			{
				int status;
				waitpid(pids[i], &status, 0);
			}
		}
		else
		{
			shellerror();
			continue;
		}
	}

	return 0;
}

