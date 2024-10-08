// shell_commander.h
#ifndef SHELL_COMMANDER_H
#define SHELL_COMMANDER_H

#include <iostream>
#include <string>
#include <vector>

class ShellCommander {
public:
    void run();
    void echo(const std::vector<std::string>& args);
    void pwd();
    void ls(const std::vector<std::string>& args);
    void executeCommand(const std::vector<std::string>& args);
    void pipeCommands(const std::vector<std::string>& args);
    void redirectInput(const std::vector<std::string>& args);
    void redirectOutput(const std::vector<std::string>& args);
};

#endif  // SHELL_COMMANDER_H