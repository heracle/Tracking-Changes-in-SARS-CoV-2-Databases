#include "logger.hpp"

#include <iostream>

bool Logger::verbose = false;

void Logger::trace(const std::string &msg) {
    if (Logger::verbose) {
        std::cout << "TRACE -> " << msg << "\n";
    }
}

void Logger::warn(const std::string &msg) {
    std::cout << "WARN -> " << msg << "\n";
}

void Logger::error(const std::string &msg) {
    std::cerr << "ERROR -> " << msg << "\n" << std::endl;
    exit(1);
}

void Logger::set_verbose(const bool vb) {
    Logger::verbose = vb;
}
