#include "../include/shell_commander.h"
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void ShellCommander::pwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    std::cout << "Current working directory: " << cwd << std::endl;
}

void ShellCommander::ls() {
    std::string command = "ls";
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execlp(command.c_str(), command.c_str(), (char*)NULL);
        std::cerr << "Error executing command: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        // Parent process
        waitpid(pid, NULL, 0);
    }
}

void ShellCommander::executeCommand(const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execlp(args[0].c_str(), args[0].c_str(), (char*)NULL);
        std::cerr << "Error executing command: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        // Parent process
        waitpid(pid, NULL, 0);
    }
}

void ShellCommander::pipeCommands(const std::vector<std::string>& args) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Error creating pipe: " << strerror(errno) << std::endl;
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Child process 1
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]); // Close write end

        std::vector<std::string> cmd1Args;
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "|") break;
            cmd1Args.push_back(args[i]);
        }
        executeCommand(cmd1Args);
    } else {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            // Child process 2
            close(pipefd[1]); // Close write end
            dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
            close(pipefd[0]); // Close read end

            std::vector<std::string> cmd2Args;
            size_t j = 0; // declare j here
            for (size_t i = 0; i < args.size(); ++i) {
                if (args[i] == "|") {
                    j = i + 1; // assign the value of i to j
                    break;
                }
            }
            for (; j < args.size(); ++j) {
                cmd2Args.push_back(args[j]);
            }
            executeCommand(cmd2Args);
        } else {
            // Parent process
            close(pipefd[0]); // Close read end
            close(pipefd[1]); // Close write end
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}

void ShellCommander::redirectOutput(const std::vector<std::string>& args) {
    std::string outputFile;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">") {
            outputFile = args[i + 1];
            break;
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT, 0644);
        if (fd == -1) {
            std::cerr << "Error opening output file: " << strerror(errno) << std::endl;
            exit(1);
        }
        dup2(fd, STDOUT_FILENO); // Redirect stdout to file
        close(fd); // Close file descriptor

        std::vector<std::string> cmdArgs;
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == ">") break;
            cmdArgs.push_back(args[i]);
        }
        executeCommand(cmdArgs);
    } else {
        // Parent process
        waitpid(pid, NULL, 0);
    }
}

void ShellCommander::run() {
    std::string command;
    std::string currentDir = getcwd(NULL, 0);
    while (true) {
        std::cout << "shell_commander> ";
        std::getline(std::cin, command);
        if (command == "exit") {
            break;
        }

        std::istringstream iss(command);
        std::vector<std::string> args;
        std::string arg;
        while (iss >> arg) {
            args.push_back(arg);
        }

        if (args[0] == "cd") {
            if (args.size() < 2) {
                std::cerr << "Error: cd requires a directory" << std::endl;
                continue;
            }

            std::string dir = args[1];
            if (dir == "~") {
                dir = getenv("HOME");
            }

            if (chdir(dir.c_str()) != 0) {
                std::cerr << "Error changing directory: " << strerror(errno) << std::endl;
            }
        } else if (args[0] == "pwd") {
            pwd();
        } else if (args[0] == "ls") {
            ls();
        } else {
            bool pipe = false;
            bool redirect = false;
            for (const auto& arg : args) {
                if (arg == "|") pipe = true;
                if (arg == ">") redirect = true;
            }

            if (pipe) {
                pipeCommands(args);
            } else if (redirect) {
                redirectOutput(args);
            } else {
                executeCommand(args);
            }
        }
    }
}

int main() {
    ShellCommander commander;
    commander.run();
    return 0;
}