# ShellCommander

## Overview

**ShellCommander** is a custom shell (command-line interpreter) implemented in C++ that supports basic Unix-style commands, piping, input/output redirection, background task execution, and signal handling. The shell is designed to provide functionality similar to standard Unix shells but simplified for educational and learning purposes. 

This project demonstrates key concepts in process management, system calls, I/O redirection, and Unix pipelines while also showcasing advanced features like job control for background tasks and handling signals such as `Ctrl+C`.

## Features

### 1. **Basic Command Execution**
ShellCommander supports the execution of standard commands like:
- `ls` - List directory contents
- `pwd` - Print current working directory
- `cd` - Change directory
- `echo` - Display a line of text

### 2. **Piping (`|`)**
The shell can connect multiple commands together through Unix pipes. For example:
```bash
ls -l | grep "file"
```
This feature allows the output of one command to be used as input for another.

### 3. **I/O Redirection (`>`, `<`)**
ShellCommander supports input and output redirection:
- Redirect output to a file: `command > output.txt`
- Redirect input from a file: `command < input.txt`
  
This enables the shell to work with files as input/output streams for commands.

### 4. **Background Task Execution (`&`)**
Commands can be run in the background, allowing the user to continue interacting with the shell while tasks execute in the background:
```bash
command &
```
The shell handles the management of background processes, including displaying notifications when a background process finishes.

### 5. **Signal Handling**
ShellCommander captures and handles Unix signals like `SIGINT` (triggered by `Ctrl+C`), ensuring the shell remains responsive and can properly terminate or interrupt tasks without crashing.

### 6. **Job Control (Bonus)**
Users can manage and control running jobs, pausing and resuming background tasks using features like:
- Listing background jobs
- Stopping, resuming, or bringing jobs to the foreground

## Technologies Used

- **Language**: C++
- **System Calls**: `fork()`, `exec()`, `wait()`, `dup2()`
- **Process Management**: Background tasks, job control
- **File I/O**: Redirection, piping
- **Signal Handling**: `SIGINT`, `SIGCHLD`, etc.

## How to Run

1. **Clone the repository**:
   ```bash
   git clone https://github.com/CKShetty4/ShellCommander.git
   ```
2. **Navigate to the project directory**:
   ```bash
   cd ShellCommander
   ```
3. **Build the shell**:
   ```bash
   make
   ```
4. **Run the shell**:
   ```bash
   ./shell_commander
   ```

## Usage

Once inside ShellCommander, you can type standard Unix commands such as:
```bash
ls -l
pwd
cd /path/to/directory
```

Use piping and redirection:
```bash
ls -l | grep shell > output.txt
cat < input.txt | sort
```

Run tasks in the background:
```bash
sleep 10 &
```

## Key Concepts Demonstrated

- **Process Management**: Creating and managing child processes using `fork()` and `exec()`.
- **System Calls**: Using system-level calls to execute commands, handle input/output, and manage processes.
- **I/O Redirection**: Redirecting input and output streams to files or other processes.
- **Pipelines**: Connecting multiple processes via pipes for efficient data transfer.
- **Job Control and Signals**: Running tasks in the background and handling signals for process control.

## Future Enhancements

- Implementing support for advanced job control commands like `jobs`, `fg`, and `bg`.
- Adding tab-completion for commands and file paths.
- Supporting more complex command chains and error handling.
