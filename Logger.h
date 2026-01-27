//
// Created by dunamis on 26/01/2026.
//

#ifndef SMARTDRIVE_LOGGER_H
#define SMARTDRIVE_LOGGER_H

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    using LogCallback = void (*)(LogLevel level, const char* message);

    static void setCallback(LogCallback cb) {
        getCallback() = cb;
    }

    static void log(LogLevel level, const char* message) {
        if (getCallback()) getCallback()(level, message);
    }
private:
    static LogCallback& getCallback() {
        static LogCallback instance = nullptr;
        return instance;
    }

};

#endif //SMARTDRIVE_LOGGER_H