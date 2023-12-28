# Custom Shell Project
This project is an implementation of a custom shell in C, designed to provide users with a simple yet functional command-line interface. The shell supports basic command execution, input/output redirection, piped commands, and the ability to run commands in parallel or sequentially. The code is structured to facilitate understanding and modification while adhering to best practices for readability.

## Features:
1. Basic Commands: Execute single commands with or without options. <br>
2. Redirection: Redirect command output to a file using > symbol. <br>
3. Parallel Execution: Run multiple commands in parallel using the **&&** symbol.
4. Sequential Execution: Execute commands sequentially using the **##** symbol.
5. Piped Execution: Execute piped commands using the **|** symbol.
6. Signal Handling: Proper handling of signals like SIGINT and SIGTSTP.
Instructions:
Compilation:

Compile the code using a C compiler (e.g., gcc -o myshell myshell.c).
Execution:

Run the compiled binary (e.g., ./myshell).
Use basic commands, redirection, or parallel/sequential execution as needed.
## Example Usage:
bash
Copy code
$ ./myshell <br>
/home/user$ ls<br>
file1.txt file2.txt<br>
/home/user$ ls && pwd && echo "Hello World"<br>
file1.txt file2.txt<br>
/home/user<br>
Hello World<br>
/home/user$ cat input.txt > output.txt<br>
/home/user$ ls | wc<br>
      2       2       18<br>
