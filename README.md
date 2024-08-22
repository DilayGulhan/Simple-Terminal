# Simple-Terminal

This C program implements a simple terminal emulator that supports various shell-like functionalities. The emulator is capable of executing commands, handling piping, background processes, and maintaining a command history. Below is an overview of its features and usage.

## Features
### Command Execution: 
Executes user commands including built-in commands like cd, pwd, history, and exit.
### History Management: 
Keeps a history of the last 10 commands entered by the user. The history is accessible using the history command.
### Pipe Handling: 
Supports piping between two commands using the | operator.
### Background Execution: 
Commands ending with & are executed in the background.
### Error Handling: 
Provides feedback on errors encountered during command execution.
### Signal Handling:
Handles SIGINT and SIGQUIT signals for background processes.

### Building
To build the program, use:
gcc -o terminal_emulator terminal_emulator.c
