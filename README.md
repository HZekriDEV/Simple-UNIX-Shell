# Simple-UNIX-Shell
This is a very simple UNIX shell written in C for the Operating Systems course at the University of South Florida. It runs most of the basic LINUX/UNIX shell commands and can be compiled by dropping both the sh.c and Makefile into a LINUX/UNIX directory and typing ```make``` followed by ```./sh```.

The shell uses a path variable to search for executables, starting with an initial path that includes /bin. It checks for executable files using the access() system call and reports an error if the command is not found in the specified paths. The shell implements three built-in commands: exit, which exits the shell with exit(0) and reports an error if arguments are passed; cd, which changes the directory using chdir() and reports an error if chdir() fails or if the number of arguments is not one; and path, which modifies the search path and allows for an empty path, meaning only built-in commands can be executed.

Output redirection is supported using the > character, redirecting output to the specified file and overwriting it if it exists. An error is reported if the file cannot be opened. Redirection is not supported for built-in commands. The shell also allows for the execution of commands in parallel using the & character and waits for all parallel commands to complete before returning control to the user.

![image](https://github.com/HZekriDEV/Simple-UNIX-Shell/assets/62521050/9291331e-548b-4c18-bf65-b3f389001c85)
