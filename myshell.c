// -*-*-*-*-**-*-*--*-*--*-*-*---*-*-*-*-*-*---*-**--*-*-*- Including Libraries *-*-*--*-*-*-*-*-*-*-*--*-*-**-*--*-*-**--*-*-**-*-
#include <stdio.h>    // -> IO
#include <string.h>   // -> String Utility Functions
#include <sys/wait.h> // -> wait()
#include <stdlib.h>   // -> exit()
#include <fcntl.h>    // -> close(), open()
#include <unistd.h>   // -> fork(), getpid(), exec()
#include <signal.h>   // -> signal()

#define TOTAL_TOKENS 400

// -*-*-*-*-**-*-*--*-*--*-*-*---*-*-*-*-*-*---*-**--*-*-*- Function Declarations *-*-*--*-*-*-*-*-*-*-*--*-*-**-*--*-*-**--*-*-**-*-

void parseInput(char *input_Line, char *command_Line_Tokens[TOTAL_TOKENS], int *isRedirected, int *isSequential, int *isParallel, int *isPiped);
void executeCommand(char *command_Line_Tokens[TOTAL_TOKENS]);
void executeParallelCommands(char *command_Line_Tokens[TOTAL_TOKENS]);
void executeSequentialCommands(char *command_Line_Tokens[TOTAL_TOKENS]);
void executeCommandRedirection(char *command_Line_Tokens[TOTAL_TOKENS]);
void executePipedCommands(char *command_Line_Tokens[TOTAL_TOKENS]);

void sigint_handler(int signum)
{
    printf("Received SIGINT(Hello) (Ctrl+C). Exiting...\n");
    exit(signum);
}
void sigtstp_handler(int signum)
{
    printf("Received SIGTSTP.(Hello) Exiting...\n");
    exit(signum);
}

// -*-*-*-*-**-*-*--*-*--*-*-*---*-*-*-*-*-*---*-**--*-*-*- Main Function *-*-*--*-*-*-*-*-*-*-*--*-*-**-*--*-*-**--*-*-**-*-

int main()
{
    // Ignoring Default Signals To Prevent Process Interruption
    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    char *command_Line_Tokens[TOTAL_TOKENS];
    char *input_Line;
    char *current_Working_Directory;
    size_t len = 0; // int gives Warning

    while (1)
    {

        /*  The getcwd() function dynamically allocates the buffer for the current working directory.
            When the size is set to zero, the buffer is allocated to fit the necessary size. */
        current_Working_Directory = getcwd(NULL, 0);
        // Display the prompt in the following format: currentWorkingDirectory$
        printf("%s$", current_Working_Directory);
        free(current_Working_Directory);

        // read input with getline function
        getline(&input_Line, &len, stdin);

        int isParallel = 0;
        int isSequential = 0;
        int isRedirected = 0;
        int isPiped = 0;

        // Parse input
        parseInput(input_Line, command_Line_Tokens, &isRedirected, &isSequential, &isParallel, &isPiped);

        // When user uses exit command.
        if (strcmp(command_Line_Tokens[0], "exit") == 0)
        {
            printf("Exiting shell...\n");
            break;
        }

        // Execute user commands based on different cases
        if (command_Line_Tokens != NULL && command_Line_Tokens[0] != NULL && strcmp(command_Line_Tokens[0], "") != 0)
        {
            // Invoke this function when the user intends to run multiple commands in parallel (commands separated by &&)
            if (isParallel == 1)
                executeParallelCommands(command_Line_Tokens);
            // Invoke this function when the user intends to run multiple commands sequentially (commands separated by ##)
            else if (isSequential == 1)
                executeSequentialCommands(command_Line_Tokens);
            // Invoke this function when the user intends to redirect the output of a single command to an output file specified by them
            else if (isRedirected == 1)
                executeCommandRedirection(command_Line_Tokens);
            // executes piped commands
            else if (isPiped == 1)
                executePipedCommands(command_Line_Tokens);
            // Invoke this function when the user intends to run a single command
            else
                executeCommand(command_Line_Tokens);
        }
    }
    return 0;
}

// -*-*-*-*-**-*-*--*-*--*-*-*---*-*-*-*-*-*---*-**--*-*-  Command Parser *-*-*--*-*-*-*-*-*-*-*--*-*-**-*--*-*-**--*-*-**-*-

/* parseInput function takes input as user command string and also pointers to where we need to parse and store data
tokens as a 2d char array to store tokens seperated from input_Line by delimiter " " using strsep function
## or && or > token is searched, if found its respective flag is set for appropriate execution in main.*/
void parseInput(char *input_Line, char *command_Line_Tokens[TOTAL_TOKENS], int *isRedirected, int *isSequential, int *isParallel, int *isPiped)
{
    // This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
    char *token;
    int i = 0;
    int tokenStarted = 0; // Flag to track when a token starts

    // Skip leading spaces
    while (*input_Line == ' ')
        input_Line++;

    // Extracting tokens by spaces
    while (*input_Line != '\0')
    {
        if (*input_Line == ' ' && tokenStarted)
        {
            tokenStarted = 0;
            *input_Line = '\0'; // Mark the end of the token
        }
        else if (*input_Line != ' ' && !tokenStarted)
        {
            tokenStarted = 1;
            command_Line_Tokens[i] = input_Line;
            i++;
        }
        input_Line++;
    }

    // Setting Flags based on the Tokens
    for (int j = 0; j < i; j++)
    {
        if (strcmp(command_Line_Tokens[j], ">") == 0)
            *isRedirected = 1;
        if (strcmp(command_Line_Tokens[j], "&&") == 0)
            *isParallel = 1;
        if (strcmp(command_Line_Tokens[j], "##") == 0)
            *isSequential = 1;
        if (strcmp(command_Line_Tokens[j], "|") == 0)
            *isPiped = 1;
    }

    if (i > 0)
    {
        command_Line_Tokens[i - 1] = strsep(&command_Line_Tokens[i - 1], "\n"); // to remove NEWLINE character from the last token.
    }
    command_Line_Tokens[i] = NULL; // Adding NULL for no option commands and to iterate through multiple command tokens.
}

// -*-*-*-*-**-*-*--*-*--*-*-*---*-*-*-*-*-*---*-**--*-*-*- Command Execution Functions *-*-*--*-*-*-*-*-*-*-*--*-*-**-*--*-*-**--*-*-**-*-

void executeCommand(char *command_tokens[TOTAL_TOKENS])
{
    if (strcmp(command_tokens[0], "cd") == 0)
    {
        // Handle cd command without forking; using chdir to change the directory
        if (command_tokens[1] == NULL)
            chdir(getenv("HOME")); // Change path to HOME directory
        else
        {
            int chdir_status = chdir(command_tokens[1]);
            if (chdir_status == -1)
                printf("bash: cd: %s: No such file or directory\n", command_tokens[1]);
        }
    }
    else
    {
        int fork_id = fork();
        if (fork_id < 0)
            exit(1);
        else if (fork_id > 0)
        {
            // Parent Process
            wait(0);
        }
        else if (fork_id == 0)
        {
            // Child Process
            // Restore signals to default handling
            signal(SIGINT, sigint_handler);
            signal(SIGTSTP, sigtstp_handler);

            char *path = command_tokens[0];
            if (execvp(path, command_tokens) == -1)
            {
                // Runs only if execvp fails
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
    }
}

/* This function is designed to execute multiple commands, each with or without options, in a parallel manner.
The commands are separated by the && token. The parent process waits until all child processes have terminated,
ensuring the parallel execution of commands provided by the user. */
void executeParallelCommands(char *command_tokens[TOTAL_TOKENS])
{
    int i = 0;
    while (command_tokens[i] != NULL)
    {
        char *executable_tokens[TOTAL_TOKENS];
        int j = 0;

        // Create separate executable tokens for each command
        while (command_tokens[i] != NULL && strcmp(command_tokens[i], "&&") != 0)
            executable_tokens[j++] = command_tokens[i++];

        if (command_tokens[i] != NULL)
            i++;

        executable_tokens[j] = NULL;

        if (strcmp(executable_tokens[0], "cd") == 0)
        {
            // Handle cd command without forking; using chdir to change the directory
            if (executable_tokens[1] == NULL)
            {
                chdir(getenv("HOME")); // Change path to HOME directory
            }
            else
            {
                int chdir_status = chdir(executable_tokens[1]);
                if (chdir_status == -1)
                    printf("bash: cd: %s: No such file or directory\n", executable_tokens[1]);
            }
        }
        else
        {
            int fork_id = fork();
            if (fork_id < 0)
                exit(1);
            else if (fork_id == 0)
            {
                // Restore signals to default handling
                signal(SIGINT, sigint_handler);
                signal(SIGTSTP, sigtstp_handler);

                char *path = executable_tokens[0];
                execvp(path, executable_tokens);

                // Runs only if execvp fails
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
    }

    // Wait for Parent Process until all Child Processes Terminate
    int status;
    int id_wait;
    while ((id_wait = waitpid(WAIT_ANY, &status, WUNTRACED)) > 0)
    {
        if (WIFSTOPPED(status))
        {
            if (WSTOPSIG(status) == SIGTSTP)
                kill(id_wait, SIGKILL); // SIGTSTP is manually handled to terminate process from stopped mode using SIGKILL
        }
    }
}

/* This Function is to execute multiple commands with/without options in sequential way seperated by token ##.
Parent waits untill childs termination gurantees serial execution of commands from user input. */
void executeSequentialCommands(char *command_tokens[TOTAL_TOKENS])
{
    int i = 0;
    while (command_tokens[i] != NULL)
    {
        char *executable_tokens[TOTAL_TOKENS];
        int j = 0;

        // Extract individual commands
        while (command_tokens[i] != NULL && strcmp(command_tokens[i], "##") != 0)
            executable_tokens[j++] = command_tokens[i++];

        if (command_tokens[i] != NULL)
            i++;

        executable_tokens[j] = NULL;

        // No forking for cd command, using chdir to change the directory
        if (strcmp(executable_tokens[0], "cd") == 0)
        {
            // Handle cd command without forking; chdir will change path
            if (executable_tokens[1] == NULL)
                chdir(getenv("HOME")); // Change path to HOME directory.
            else
            {
                int chdir_status = chdir(executable_tokens[1]);
                if (chdir_status == -1)
                    printf("bash: cd: %s: No such file or directory\n", executable_tokens[1]);
            }
        }
        else
        {
            int fork_id = fork();
            if (fork_id < 0)
                exit(1);
            else if (fork_id > 0)
            {
                wait(0);
            }
            else if (fork_id == 0)
            { // Child Process
                // Restore signals to default handling
                signal(SIGINT, sigint_handler);
                signal(SIGTSTP, sigtstp_handler);

                char *path = executable_tokens[0];
                execvp(path, executable_tokens);

                // Executed if execvp fails
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
    }
}

/* This Function is to execute only single commands with/without options with redirection
New child is forked, File descriptors are changed to output to a file as per command token. */
void executeCommandRedirection(char *command_tokens[TOTAL_TOKENS])
{
    char *executable_tokens[TOTAL_TOKENS];
    int i = 0, j = 0;

    // Extract command to be executed (tokens before >)
    while (strcmp(command_tokens[i], ">") != 0)
        executable_tokens[j++] = command_tokens[i++];

    i++;                         // Skip the ">" token
    executable_tokens[j] = NULL; // Terminate the executable_tokens array

    if (strcmp(executable_tokens[0], "cd") == 0)
    {
        // Handle cd command without forking; using chdir to change the directory
        if (executable_tokens[1] == NULL)
            chdir(getenv("HOME")); // Change path to HOME directory
        else
        {
            int chdir_status = chdir(executable_tokens[1]);
            if (chdir_status == -1)
                printf("bash: cd: %s: No such file or directory\n", executable_tokens[1]);
        }
    }
    else
    {
        int fork_id = fork();
        if (fork_id < 0)
            exit(1);
        else if (fork_id > 0)
        {
            // Parent Process
            wait(0);
        }
        else
        { // Child Process
            // Restore signals to default handling
            signal(SIGINT, sigint_handler);
            signal(SIGTSTP, sigtstp_handler);

            // Open file for redirection and close standard output
            close(STDOUT_FILENO);
            open(command_tokens[i], O_CREAT | O_WRONLY | O_APPEND, 0666);

            char *path = executable_tokens[0];
            execvp(path, executable_tokens);

            // Runs only if execvp fails
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }
}

// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- PIPE *-*-*-*-*-*-*-*-*-*-*--*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-

void executePipedCommands(char *command_Line_Tokens[TOTAL_TOKENS])
{
    int num_commands = 0;        // Counter for number of commands in the pipeline
    int i = 0;                   // Index for iterating through command tokens
    int input_fd = STDIN_FILENO; // File descriptor for input, initially standard input

    // Loop through command tokens until NULL terminator
    while (command_Line_Tokens[i] != NULL)
    {
        if (strcmp(command_Line_Tokens[i], "|") == 0)
        {
            // Found a pipe symbol, execute the current command
            if (num_commands == 0)
            {
                printf("Shell: Incorrect command\n");
                return;
            }

            // Create a pipe
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1)
            {
                perror("pipe");
                exit(1);
            }

            // Fork a child process
            pid_t pid = fork();
            if (pid == 0)
            {
                // Child process

                // Redirect input from previous command
                if (dup2(input_fd, STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(1);
                }

                // Redirect output to the pipe
                if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(1);
                }

                // Close unused pipe ends
                close(pipe_fd[0]);
                close(pipe_fd[1]);

                // Extract command arguments for current command
                char *command_args[TOTAL_TOKENS];
                int arg_count = 0;
                int j = i - num_commands;
                while (j < i)
                {
                    command_args[arg_count++] = command_Line_Tokens[j];
                    j++;
                }
                command_args[arg_count] = NULL;

                // Handle built-in command 'cd'
                if (strcmp(command_args[0], "cd") == 0)
                {
                    if (command_args[1] == NULL)
                    {
                        printf("Shell: Incorrect command\n");
                    }
                    else
                    {
                        if (chdir(command_args[1]) != 0)
                        {
                            printf("Shell: Incorrect command\n");
                        }
                    }
                    exit(0);
                }
                else
                {
                    // Execute external command
                    execvp(command_args[0], command_args);
                    perror("execvp");
                    exit(1);
                }
            }
            else if (pid > 0)
            {
                // Parent process
                close(pipe_fd[1]);     // Close write end of the pipe
                waitpid(pid, NULL, 0); // Wait for child process to complete

                input_fd = pipe_fd[0]; // Set input for the next command to read end of the pipe
                num_commands = 0;      // Reset command count for next iteration
            }
            else
            {
                perror("fork");
                exit(1);
            }
        }
        else
        {
            // Token is not a pipe symbol, increment the number of commands
            num_commands++;
        }
        i++;
    }

    // Execute the final command in the pipeline
    if (num_commands > 0)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process

            // Redirect input from previous command
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                perror("dup2");
                exit(1);
            }

            // Extract command arguments for final command
            char *command_args[TOTAL_TOKENS];
            int arg_count = 0;
            int j = i - num_commands;
            while (j < i)
            {
                command_args[arg_count++] = command_Line_Tokens[j];
                j++;
            }
            command_args[arg_count] = NULL;

            // Handle built-in command 'cd'
            if (strcmp(command_args[0], "cd") == 0)
            {
                if (command_args[1] == NULL)
                {
                    printf("Shell: Incorrect command\n");
                }
                else
                {
                    if (chdir(command_args[1]) != 0)
                    {
                        printf("Shell: Incorrect command\n");
                    }
                }
                exit(0);
            }
            else
            {
                // Execute external command
                execvp(command_args[0], command_args);
                perror("execvp");
                exit(1);
            }
        }
        else if (pid > 0)
        {
            // Parent process
            waitpid(pid, NULL, 0); // Wait for child process to complete
        }
        else
        {
            perror("fork");
            exit(1);
        }
    }
}

// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- END *-*-*-*-*-*-*-*-*-*-*--*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-

/*
ls
ls && ls && pwd && ls
ls ## ls ## pwd ## ls
df -h > x.txt
ls | wc | wc

cat main.c | grep -v "^$" | wc -l
cat main.c | grep open | wc
*/
