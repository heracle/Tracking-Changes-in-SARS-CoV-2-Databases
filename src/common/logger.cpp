#include "logger.hpp"

#include <iostream>

namespace common {

void Logger::trace(const std::string &msg) {
    std::cout << "TRACE -> " << msg << "\n";
}

void Logger::error(const std::string &msg) {
    std::cerr << "ERROR -> " << msg << "\n" << std::endl;
    exit(1);
}

void Logger::set_verbose(const bool vb) {
    verbose = vb;
}

} // namespace common
