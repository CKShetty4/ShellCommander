#include "../include/shell_commander.h"
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void ShellCommander::pwd() {
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  std::cout << "Current working directory: " << cwd << std::endl;
}

void ShellCommander::echo(const std::vector<std::string> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    std::cout << args[i] << (i < args.size() - 1 ? " " : "");
  }
  std::cout << std::endl;
}


void ShellCommander::ls(const std::vector<std::string> &args) {
  std::vector<char *> lsArgs;
  lsArgs.push_back(const_cast<char *>("ls"));
  for (const auto &arg : args) {
    if (arg == "|")
      break; // Stop adding args when we reach the pipe
    lsArgs.push_back(const_cast<char *>(arg.c_str()));
  }
  lsArgs.push_back(nullptr);

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    execvp(lsArgs[0], lsArgs.data());
    std::cerr << "Error executing command: " << strerror(errno) << std::endl;
    exit(1);
  } else {
    // Parent process
    waitpid(pid, NULL, 0);
  }
}

void ShellCommander::executeCommand(const std::vector<std::string> &args) {
  bool background = false;
  std::vector<std::string> argsCopy = args; // Make a copy of args

  if (!argsCopy.empty() && argsCopy.back() == "&") {
    background = true;
    argsCopy.pop_back();
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    execlp(argsCopy[0].c_str(), argsCopy[0].c_str(), (char *)NULL);
    std::cerr << "Error executing command: " << strerror(errno) << std::endl;
    exit(1);
  } else {
    // Parent process
    if (!background) {
      waitpid(pid, NULL, 0);
    } else {
      std::cout << "Background process started with PID: " << pid << std::endl;
    }
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
        // Child process 1 (ls)
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd[1]); // Close write end

        // Collect first command arguments (before the pipe)
        std::vector<std::string> cmd1Args;
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "|") break;
            cmd1Args.push_back(args[i]);
        }

        // Convert cmd1Args to char* array and execute it
        std::vector<char*> argv1;
        for (auto &arg : cmd1Args) argv1.push_back(const_cast<char*>(arg.c_str()));
        argv1.push_back(nullptr);

        execvp(argv1[0], argv1.data());
        std::cerr << "Error executing command: " << strerror(errno) << std::endl;
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Child process 2 (grep)
        close(pipefd[1]); // Close write end
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
        close(pipefd[0]); // Close read end

        // Collect second command arguments (after the pipe)
        std::vector<std::string> cmd2Args;
        bool pipeFound = false;
        for (size_t i = 0; i < args.size(); ++i) {
            if (pipeFound) cmd2Args.push_back(args[i]);
            if (args[i] == "|") pipeFound = true;
        }

        // Convert cmd2Args to char* array and execute it
        std::vector<char*> argv2;
        for (auto &arg : cmd2Args) argv2.push_back(const_cast<char*>(arg.c_str()));
        argv2.push_back(nullptr);

        execvp(argv2[0], argv2.data());
        std::cerr << "Error executing command: " << strerror(errno) << std::endl;
        exit(1);
    }

    // Parent process
    close(pipefd[0]); // Close read end
    close(pipefd[1]); // Close write end
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void ShellCommander::redirectInput(const std::vector<std::string> &args) {
  std::string inputFile;
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "<") {
      inputFile = args[i + 1];
      break;
    }
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) {
      std::cerr << "Error opening input file: " << strerror(errno) << std::endl;
      exit(1);
    }
    dup2(fd, STDIN_FILENO); // Redirect stdin to file
    close(fd);              // Close file descriptor

    std::vector<std::string> cmdArgs;
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i] == "<") break;
      cmdArgs.push_back(args[i]);
    }
    executeCommand(cmdArgs);
  } else {
    // Parent process
    waitpid(pid, NULL, 0);
  }
}


void ShellCommander::redirectOutput(const std::vector<std::string> &args) {
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
      std::cerr << "Error opening output file: " << strerror(errno)
                << std::endl;
      exit(1);
    }
    dup2(fd, STDOUT_FILENO); // Redirect stdout to file
    close(fd);               // Close file descriptor

    std::vector<std::string> cmdArgs;
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i] == ">")
        break;
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
      // Handle 'cd' command
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
      std::vector<std::string> lsArgs(args.begin() + 1, args.end());
      if (lsArgs.empty()) {
        ls({"-l"}); // default to ls -l if no options are given
      } else {
        ls(lsArgs);
      }
    } else if (args[0] == "echo") {
      echo(args);
    } else {
      bool isPipe = false;
      bool isRedirectOutput = false;
      bool isRedirectInput = false;

      for (const auto &arg : args) {
        if (arg == "|") isPipe = true;
        if (arg == ">") isRedirectOutput = true;
        if (arg == "<") isRedirectInput = true;
      }

      if (isPipe) {
        pipeCommands(args);
      } else if (isRedirectOutput) {
        redirectOutput(args);
      } else if (isRedirectInput) {
        redirectInput(args);
      } else {
        executeCommand(args);
      }
    }
  }
}


void signalHandler(int sig) {
  if (sig == SIGINT) {
    std::cout << "Received SIGINT signal. Exiting..." << std::endl;
    exit(0);
  }
}

int main() {
  signal(SIGINT, signalHandler);
  ShellCommander commander;
  commander.run();
  return 0;
}