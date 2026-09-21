#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <windows.h>

namespace TogetherSpawn {
namespace Utils {

    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    class Logger {
    public:
        static void Log(LogLevel level, const std::string& message) {
            std::string prefix;
            switch (level) {
                case LogLevel::Debug:   prefix = "[TogetherSpawn][DEBUG] "; break;
                case LogLevel::Info:    prefix = "[TogetherSpawn][INFO] "; break;
                case LogLevel::Warning: prefix = "[TogetherSpawn][WARN] "; break;
                case LogLevel::Error:   prefix = "[TogetherSpawn][ERROR] "; break;
            }
            std::string fullMsg = prefix + message + "\n";
            OutputDebugStringA(fullMsg.c_str());
            std::cout << fullMsg;
        }

        static void Debug(const std::string& msg)   { Log(LogLevel::Debug, msg); }
        static void Info(const std::string& msg)    { Log(LogLevel::Info, msg); }
        static void Warn(const std::string& msg)    { Log(LogLevel::Warning, msg); }
        static void Error(const std::string& msg)   { Log(LogLevel::Error, msg); }
    };

} // namespace Utils
} // namespace TogetherSpawn
