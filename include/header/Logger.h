#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
#include <mutex>
#include "Config.h"

#ifdef _MSC_VER
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define LOG_TRACE(cat, msg) Logger::log(Logger::Level::TRACE, cat, msg, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define LOG_DEBUG(cat, msg) Logger::log(Logger::Level::DEBUG, cat, msg, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define LOG_INFO(cat, msg)  Logger::log(Logger::Level::INFO,  cat, msg, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define LOG_WARN(cat, msg)  Logger::log(Logger::Level::WARN,  cat, msg, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define LOG_ERROR(cat, msg) Logger::log(Logger::Level::ERROR, cat, msg, __FILE__, __PRETTY_FUNCTION__, __LINE__)

class Logger {
public:

    enum class Level {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    static void init() {
        std::lock_guard<std::mutex> lock(logMutex);

        terminalLevel = static_cast<Level>(g_config.terminalLogLevel);
        fileLogLevel = static_cast<Level>(g_config.fileLogLevel);

        std::string ts = getCurrentDateTime();
        if (g_config.debugMode) {
            mainFile.open("../logs/game_" + ts + ".jsonl");
            aiFile.open("../logs/ml_" + ts + ".jsonl");
        }
    }

    static void shutdown() {
        if (mainFile.is_open()) mainFile.close();
        if (aiFile.is_open()) aiFile.close();
    }

    static void setLogLevel(Level terminalLvl, Level fileLvl) {
        std::lock_guard<std::mutex> lock(logMutex);
        terminalLevel = terminalLvl;
        fileLogLevel = fileLvl;
    }

    static void setLogLevel(Level commonLevel) {
        std::lock_guard<std::mutex> lock(logMutex);
        terminalLevel = commonLevel;
        fileLogLevel = commonLevel;
    }

    static std::string extractClassAndMethod(const std::string& prettyFunction)
    {
        size_t start = prettyFunction.find(' ');
        if (start == std::string::npos) return prettyFunction;

        size_t end = prettyFunction.find('(');
        if (end == std::string::npos) return prettyFunction;

        std::string signature = prettyFunction.substr(start + 1, end - start - 1);

        return signature;
    }

    template<typename T>
    static void log(Level level, const std::string& category, const T& msg,
                    const std::string& file, const std::string& function, int line)
    {
        if (level < terminalLevel && level < fileLogLevel) return;

        std::string msgStr = toString(msg);
        std::lock_guard<std::mutex> lock(logMutex);

        if (level >= terminalLevel) {
            std::ostringstream readable;
            readable << timestamp() << " [" << levelToString(level) << "][" << category << "] " << msgStr;
            std::cout << readable.str() << std::endl;
        }

        if (level >= fileLogLevel) {
            std::string jsonEntry = buildJson(level, category, msgStr, file, function, line);
            if (category == "AI" && aiFile.is_open()) {
                aiFile << jsonEntry << std::endl;
            } else if (mainFile.is_open()) {
                mainFile << jsonEntry << std::endl;
            }
        }
    }

    template<typename T>
    static void trace(const std::string& cat, const T& msg) {
        log(Level::TRACE, cat, msg, __FILE__, __func__, __LINE__);
    }

    template<typename T>
    static void info(const std::string& cat, const T& msg) {
        log(Level::INFO, cat, msg, __FILE__, __func__, __LINE__);
    }

    template<typename T>
    static void debug(const std::string& cat, const T& msg) {
        log(Level::DEBUG, cat, msg, __FILE__, __func__, __LINE__);
    }

    template<typename T>
    static void warn(const std::string& cat, const T& msg) {
        log(Level::WARN, cat, msg, __FILE__, __func__, __LINE__);
    }

    template<typename T>
    static void error(const std::string& cat, const T& msg) {
        log(Level::ERROR, cat, msg, __FILE__, __func__, __LINE__);
    }

private:
    static inline std::ofstream mainFile;
    static inline std::ofstream aiFile;
    static inline std::mutex logMutex;

    static inline Level terminalLevel = Level::INFO;
    static inline Level fileLogLevel = Level::DEBUG;

    static std::string buildJson(Level level,
                             const std::string& category,
                             const std::string& msg,
                             const std::string& file,
                             const std::string& function,
                             int line)
    {
        std::ostringstream json;

        json << "{"
            << "\"time\":\"" << timestamp() << "\","
            << "\"level\":\"" << levelToString(level) << "\","
            << "\"category\":\"" << category << "\","
            << "\"trigger\":\"" << function << "\","
            << "\"file\":\"" << getFileName(file) << "\","
            << "\"line\":" << line << ","
            << "\"msg\":\"" << escapeJson(msg) << "\""
            << "}";

        return json.str();
    }

    template<typename T>
    static std::string toString(const T& msg) {
        std::ostringstream oss;
        oss << msg;
        return oss.str();
    }

    static std::string getFileName(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos == std::string::npos) return path;
        return path.substr(pos + 1);
    }

    static std::string escapeJson(const std::string& s) {
        std::string out;
        for (char c : s) {
            switch (c) {
                case '\"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out += c;
            }
        }
        return out;
    }

    static std::string levelToString(Level lvl) {
        switch (lvl) {
            case Level::TRACE: return "TRACE";
            case Level::DEBUG: return "DEBUG";
            case Level::INFO:  return "INFO";
            case Level::WARN:  return "WARN";
            case Level::ERROR: return "ERROR";
        }
        return "UNKNOWN";
    }

    static std::string timestamp() {
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
        return buf;
    }

    static std::string getCurrentDateTime() {
        std::time_t now = std::time(nullptr);
        std::tm* t = std::localtime(&now);

        std::ostringstream oss;
        oss << (t->tm_year + 1900) << "-"
            << (t->tm_mon + 1) << "-"
            << t->tm_mday << "_"
            << t->tm_hour << "-"
            << t->tm_min << "-"
            << t->tm_sec;

        return oss.str();
    }
};