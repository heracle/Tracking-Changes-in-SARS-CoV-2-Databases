#pragma once

#include <string>

namespace common {

class Logger {
    bool verbose = false;
    public:
    // Logger();
    // void flush();
    void trace(const std::string &msg);
    void error(const std::string &msg);
    void set_verbose(const bool vb);
};

} // namespace common
