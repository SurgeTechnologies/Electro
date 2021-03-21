//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/ElectroLog.hpp"
#include <cstring>
#include <ctime>
#include <filesystem>
#include <stdarg.h>
#include <stdio.h>

namespace Electro
{
    ELogger ELogger::sCoreLogger = ELogger("Electro");
    Vector<String> ELogger::sBuffer;

    bool ELogger::sLogToFile = true;
    bool ELogger::sLogToConsole = true;

    const char* ELogger::sPreviousFile = "Logs/ElectroPrevLog.elog";
    const char* ELogger::sCurrentFile = "Logs/ElectroLog.eLog";

    ELogger::ELogger(const char* name)
        : mName(name) {}

    void ELogger::ELog(Severity severity, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, severity, format, args);
        va_end(args);
    }

    void ELogger::ELogTrace(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, Severity::Trace, format, args);
        va_end(args);
    }

    void ELogger::ELogInfo(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, Severity::Info, format, args);
        va_end(args);
    }
    void ELogger::ELogDebug(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, Severity::Debug, format, args);
        va_end(args);
    }

    void ELogger::ELogWarning(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, Severity::Warning, format, args);
        va_end(args);
    }

    void ELogger::ELogError(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, Severity::Error, format, args);
        va_end(args);
    }

    void ELogger::ELogCritical(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        ELogger::ELog(mName, Severity::Critical, format, args);
        va_end(args);
    }

    void ELogger::EInit()
    {
        if (std::filesystem::exists(ELogger::sCurrentFile))
        {
            if (std::filesystem::exists(ELogger::sPreviousFile))
                std::filesystem::remove(ELogger::sPreviousFile);

            if (rename(ELogger::sCurrentFile, ELogger::sPreviousFile))
                ELogger("Logger").ELog(Severity::Debug, "Failed to rename log file %s to %s", ELogger::sCurrentFile, ELogger::sPreviousFile);
        }
    }

    void ELogger::EShutdown()
    {
        EFlush();
    }

    void ELogger::EFlush()
    {
        if (!ELogger::sLogToFile)
            return;

        std::filesystem::path filepath{ ELogger::sCurrentFile };
        std::filesystem::create_directories(filepath.parent_path());

        FILE* file = fopen(ELogger::sCurrentFile, "a");
        if (file)
        {
            for (auto message : ELogger::sBuffer)
                fwrite(message.c_str(), sizeof(char), message.length(), file);
            fclose(file);
            ELogger::sBuffer.clear();
        }
        else
        {
            ELogger::sLogToFile = false;
        }
    }

    Uint ELogger::EGetSeverityMaxBufferCount(Severity severity)
    {
        switch (severity)
        {
            case Severity::Trace:
                return 100;
            case Severity::Info:
                return 100;
            case Severity::Debug:
                return 50;
            case Severity::Warning:
                return 10;
            case Severity::Error:
                return 0;
            case Severity::Critical:
                return 0;
        }
        return 0;
    }

    const char* ELogger::EGetSeverityID(Severity severity)
    {
        switch (severity)
        {
            case Severity::Trace:
                return "TRACE";
            case Severity::Info:
                return "INFO";
            case Severity::Debug:
                return "DEBUG";
            case Severity::Warning:
                return "WARNING";
            case Severity::Error:
                return "ERROR";
            case  Severity::Critical:
                return "CRITICAL";
        }
        return "Unknown Severity";
    }

    const char* ELogger::EGetSeverityConsoleColor(Severity severity)
    {
       /*
        * Console Colors https://stackoverflow.com/questions/4053837
        * Name            FG  BG
        * Black           30  40
        * Red             31  41
        * Green           32  42
        * Yellow          33  43
        * Blue            34  44
        * Magenta         35  45
        * Cyan            36  46
        * White           37  47
        * Bright Black    90  100
        * Bright Red      91  101
        * Bright Green    92  102
        * Bright Yellow   93  103
        * Bright Blue     94  104
        * Bright Magenta  95  105
        * Bright Cyan     96  106
        * Bright White    97  107
        */
        switch (severity)
        {
            case Severity::Trace:
                return "\033[0;97m";
            case Severity::Info:
                return "\033[0;92m";
            case Severity::Debug:
                return "\033[0;36m";
            case Severity::Warning:
                return "\033[0;93m";
            case Severity::Error:
                return "\033[0;91m";
            case Severity::Critical:
                return "\033[0;101m";
        }
        return "\033[0;97m";
    }

    void ELogger::ELog(const char* name, Severity severity, const char* format, va_list args)
    {
        Uint length = vsnprintf(nullptr, 0, format, args) + 1;
        char* buf = new char[length];
        vsnprintf(buf, length, format, args);

        std::string message(buf);
        delete[] buf;

        std::vector<std::string> messages;

        Uint lastIndex = 0;
        for (Uint i = 0; i < message.length(); i++)
        {
            if (message[i] == '\n')
            {
                messages.push_back(message.substr(lastIndex, i - lastIndex));
                lastIndex = i + 1;
            }
            else if (i == message.length() - 1)
            {
                messages.push_back(message.substr(lastIndex));
            }
        }

        for (std::string msg : messages)
        {
            std::string logMsg = "";
            std::string systemConsoleMsg = "";
            std::string editorConsoleMsg = "";

            constexpr Uint timeBufferSize = 16;
            std::time_t        currentTime = std::time(nullptr);
            char               timeBuffer[timeBufferSize];

            if (ELogger::sLogToFile)
                logMsg += "[" + std::string(name) + "]";
            if (ELogger::sLogToConsole)
                systemConsoleMsg += std::string(ELogger::EGetSeverityConsoleColor(severity)) + "[" + std::string(name) + "]";

            if (std::strftime(timeBuffer, timeBufferSize, "[%H:%M:%S]", std::localtime(&currentTime)))
            {
                if (ELogger::sLogToFile)
                    logMsg += timeBuffer;
                if (ELogger::sLogToConsole)
                    systemConsoleMsg += timeBuffer;
            }

            if (ELogger::sLogToFile)
                logMsg += " " + std::string(ELogger::EGetSeverityID(severity)) + ": " + msg + "\n";
            if (ELogger::sLogToConsole)
                systemConsoleMsg += " " + std::string(ELogger::EGetSeverityID(severity)) + ": " + msg + +"\033[0m " + "\n";

            if (ELogger::sLogToFile)
                ELogger::sBuffer.push_back(logMsg);
            if (ELogger::sLogToConsole)
                printf("%s", systemConsoleMsg.c_str());
        }

        if (ELogger::sLogToFile)
            if (ELogger::sBuffer.size() > ELogger::EGetSeverityMaxBufferCount(severity))
                EFlush();
    }
}