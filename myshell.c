/********************************************************************************************
This is a template for assignment on writing a custom Shell.

Though use of any extra functions is not recommended, students may use new functions if they need to,
but that should not make code unnecessorily complex to read.

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Students need to be careful while forking a new process (no unnecessory process creations)
or while inserting the single handler code (should be added at the correct places).

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp,
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

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
    printf("Received SIGINT(HeLlO) (Ctrl+C). Exiting...\n");
    exit(signum);
}

void sigtstp_handler(int signum)
{
    printf("Received SIGTSTP.(HeLlO) Exiting...\n");
    exit(signum);
}

// -*-*-*-*-**-*-*--*-*--*-*-*---*-*-*-*-*-*---*-**--*-*-*- Main Function *-*-*--*-*-*-*-*-*-*-*--*-*-**-*--*-*-**--*-*-**-*-

// DONE
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

// DONE
/* parseInput function takes input as user command string and also pointers to where we need to parse and store data
tokens as a 2d char array to store tokens seperated from input_Line by delimiter " " using strsep function
## or && or > token is searched, if found its respective flag is set for appropriate execution in main.*/

/*
void parseInput(char *input_Line, char *command_Line_Tokens[TOTAL_TOKENS], int *isRedirected, int *isSequential, int *isParallel)
{
    // This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
    char *token;
    int i = 0;

    // Skip leading spaces
    while (*input_Line == ' ')
        input_Line++;

    // Extracting tokens by spaces
    while ((token = strsep(&input_Line, " ")) != NULL)
    {
        command_Line_Tokens[i] = token;
        // Setting Flags based on their Tokens
        if (strcmp(command_Line_Tokens[i], ">") == 0)
            *isRedirected = 1;
        if (strcmp(command_Line_Tokens[i], "&&") == 0)
            *isParallel = 1;
        if (strcmp(command_Line_Tokens[i], "##") == 0)
            *isSequential = 1;
        i++;
    }
    command_Line_Tokens[i - 1] = strsep(&command_Line_Tokens[i - 1], "\n"); // to remove NEWLINE character from command_Line_Tokens at the last character.
    command_Line_Tokens[i] = NULL;                                          // Adding NULL to for no option commands and to iterate through multiple command tokens.
    // ADDING NULL OTHERWISE EXECVP WONT WORK
}
*/

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

// DONE
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
            // execvp(path, command_tokens);
            if (execvp(path, command_tokens) == -1)
            {
                // Runs only if execvp fails
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        }
    }
}

// DONE
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

// DONE
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

// DONE
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
            /*
            // Parent Process
            int child_status;
            int child_id = waitpid(WAIT_ANY, &child_status, WUNTRACED);
            if (WIFSTOPPED(child_status))
            {
                if (WSTOPSIG(child_status) == SIGTSTP)
                    kill(child_id, SIGKILL); // Terminate process from stopped mode using SIGKILL
            }
            */
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

// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*- PIPE(Bonus Question) *-*-*-*-*-*-*-*-*-*-*--*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-

/*
void executePipedCommands(char *command_Line_Tokens[TOTAL_TOKENS])
{
    int fd[2];
    if (pipe(fd) <= 0)
    {
        exit(1);
    }

    pid_t p1, p2;
    p1 = fork();

    if (p1 < 0)
    {
        exit(1);
    }
    else if (p1 == 0)
    {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        execvp(command_Line_Tokens[0], command_Line_Tokens);

        printf("Shell: Incorrect command\n");
        exit(1);
    }
    else
    {
        p2 = fork();

        if (p2 < 0)
            exit(1);

        else if (p2 == 0)
        {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);

            // PIPE FOR SECOND COMMAND
            execvp(command_Line_Tokens[0], command_Line_Tokens);

            printf("Shell: Incorrect command\n");
            exit(1);
        }
        else
        {
            wait(0);
            wait(0);
        }
    }

    int i = 0;
    while (command_tokens[i] != NULL)
    {
        char *executable_tokens[TOTAL_TOKENS];
        int j = 0;

        // Extract individual commands
        while (command_tokens[i] != NULL && strcmp(command_tokens[i], "|") != 0)
            executable_tokens[j++] = command_tokens[i++];

        if (command_tokens[i] != NULL)
            i++;

        executable_tokens[j] = NULL;


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
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            char *path = executable_tokens[0];
            execvp(path, executable_tokens);

            // Executed if execvp fails
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    }
}
*/
/*
void executePipedCommands(char *command_Line_Tokens[TOTAL_TOKENS])
{
    int i = 0;
    int pipe_count = 0;

    // Count the number of pipes
    while (command_Line_Tokens[i] != NULL)
    {
        if (strcmp(command_Line_Tokens[i], "|") == 0)
            pipe_count++;
        i++;
    }

    // Creating an array of pipes
    int pipes[pipe_count][2];
    for (int j = 0; j < pipe_count; j++)
    {
        if (pipe(pipes[j]) < 0)
        {
            exit(1);
        }
    }

    int start = 0;
    for (int j = 0; j <= pipe_count; j++)
    {
        i = start;
        char *executable_tokens[TOTAL_TOKENS];
        int k = 0;

        // Extract individual commands
        while (command_Line_Tokens[i] != NULL && strcmp(command_Line_Tokens[i], "|") != 0)

            executable_tokens[k++] = command_Line_Tokens[i++];

        if (command_Line_Tokens[i] != NULL)
            i++;
        executable_tokens[k] = NULL;

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Fork failed");
            exit(1);
        }
        else if (pid == 0)
        { // Child Process
            // If it's not the first command, set input from the previous pipe
            if (j > 0)
            {
                close(pipes[j - 1][1]);              // Close write end of previous pipe
                dup2(pipes[j - 1][0], STDIN_FILENO); // Redirect stdin to read end of previous pipe
                close(pipes[j - 1][0]);
            }

            // If it's not the last command, set output to the current pipe
            if (j < pipe_count)
            {
                close(pipes[j][0]);               // Close read end of current pipe
                dup2(pipes[j][1], STDOUT_FILENO); // Redirect stdout to write end of current pipe
                close(pipes[j][1]);
            }

            // Restore signals to default handling
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            char *path = executable_tokens[0];
            execvp(path, executable_tokens);

            printf("Shell: Incorrect command\n");
            exit(1);
        }
        else
        { // Parent Process
            if (j > 0)
            {
                close(pipes[j - 1][0]); // Close read end of previous pipe
                close(pipes[j - 1][1]); // Close write end of previous pipe
            }

            wait(NULL); // Wait for the child to finish

            if (j == 0)
                start = i; // Move the start index for the next iteration
        }
    }
}
*/

void executePipedCommands(char *command_Line_Tokens[TOTAL_TOKENS])
{
    int num_commands = 0;
    int i = 0;
    int input_fd = STDIN_FILENO;

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

            int pipe_fd[2];
            if (pipe(pipe_fd) == -1)
            {
                perror("pipe");
                exit(1);
            }

            pid_t pid = fork();
            if (pid == 0)
            {
                // Child process
                if (dup2(input_fd, STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(1);
                }
                if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(1);
                }

                close(pipe_fd[0]);
                close(pipe_fd[1]);

                char *command_args[TOTAL_TOKENS];
                int arg_count = 0;
                int j = i - num_commands;
                while (j < i)
                {
                    command_args[arg_count++] = command_Line_Tokens[j];
                    j++;
                }
                command_args[arg_count] = NULL;

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
                    execvp(command_args[0], command_args);
                    perror("execvp");
                    exit(1);
                }
            }
            else if (pid > 0)
            {
                // Parent process
                close(pipe_fd[1]);
                waitpid(pid, NULL, 0);

                input_fd = pipe_fd[0];
                num_commands = 0;
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

    // Execute the final command
    if (num_commands > 0)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                perror("dup2");
                exit(1);
            }

            char *command_args[TOTAL_TOKENS];
            int arg_count = 0;
            int j = i - num_commands;
            while (j < i)
            {
                command_args[arg_count++] = command_Line_Tokens[j];
                j++;
            }
            command_args[arg_count] = NULL;

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
                execvp(command_args[0], command_args);
                perror("execvp");
                exit(1);
            }
        }
        else if (pid > 0)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            perror("fork");
            exit(1);
        }
    }
}

// Restore signals to

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