#include "../include/shell_commander.h"
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void ShellCommander::run() {
    std::string command;
    std::string currentDir = getcwd(NULL, 0);
    while (true) {
        std::cout << "shell_commander> ";
        std::getline(std::cin, command);
        if (command == "exit") {
            break;
        }

        std::vector<std::string> args;
        std::istringstream iss(command);
        std::string token;
        while (iss >> token) {
            args.push_back(token);
        }

        if (args[0] == "cd") {
            // Handle cd command
            if (args.size() == 2) {
                if (args[1] == "..") {
                    // Go to parent directory
                    size_t pos = currentDir.find_last_of('/');
                    if (pos != std::string::npos) {
                        currentDir = currentDir.substr(0, pos);
                    }
                } else if (args[1] == ".") {
                    // Stay in current directory
                } else if (args[1][0] == '/') {
                    // Absolute path
                    currentDir = args[1];
                } else {
                    // Relative path
                    currentDir += "/" + args[1];
                }
                if (chdir(currentDir.c_str()) != 0) {
                    std::cerr << "Error: " << strerror(errno) << std::endl;
                }
            } else {
                std::cerr << "Usage: cd <directory>" << std::endl;
            }
        } else if (args [0] == "pwd") {
            pwd();
        } else if (args[0] == "ls") {
            ls();
        } else {
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
    }
};

int main() {
    ShellCommander shell;
    shell.run();
    return 0;
}