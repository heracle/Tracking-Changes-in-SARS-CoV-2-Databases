#pragma once

#include <string>

// namespace common {

class Logger {
    static bool verbose;
  public:
    // Logger();
    // void flush();
    static void trace(const std::string &msg);
    static void error(const std::string &msg);
    static void warn(const std::string &msg);
    static void set_verbose(const bool vb);
};


// } // namespace common
