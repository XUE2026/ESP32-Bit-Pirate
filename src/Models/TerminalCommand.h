#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

class TerminalCommand {
public:
    TerminalCommand(const std::string& root = "", const std::string& sub = "", const std::string& args = "")
        : root(root), subcommand(sub), args(args) {}

    std::string getRoot() const { return root; }
    void setRoot(const std::string& r) { root = r; }

    std::string getSubcommand() const { return subcommand; }
    void setSubcommand(const std::string& s) { subcommand = s; }

    std::string getArgs() const { return args; }
    void setArgs(const std::string& a) { args = a; }

    // Returns parsed arguments as vector of strings (split by whitespace)
    std::vector<std::string> getArgsVector() const {
        std::vector<std::string> result;
        std::istringstream iss(args);
        std::string token;
        while (iss >> token) {
            result.push_back(token);
        }
        return result;
    }

private:
    std::string root;
    std::string subcommand;
    std::string args;
};
