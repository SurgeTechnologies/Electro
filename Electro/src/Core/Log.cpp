//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/Log.hpp"
#include "Panels/ConsolePanel.hpp" //TODO: Remove

namespace Electro
{
    Logger Logger::sCoreLogger = Logger("Electro");
    Vector<String> Logger::sBuffer;

    bool Logger::sLogToFile = false;
    bool Logger::sLogToConsole = true;
    bool Logger::sLogToEditorConsole = true;

    const char* Logger::sPreviousFile = "Logs/ElectroPrevLog.elog";
    const char* Logger::sCurrentFile = "Logs/ElectroLog.eLog";

    Logger::Logger(const char* name)
        : mName(name) {}

    void Logger::Log(Severity severity, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, severity, format, args);
        va_end(args);
    }

    void Logger::LogTrace(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, Severity::Trace, format, args);
        va_end(args);
    }

    void Logger::LogInfo(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, Severity::Info, format, args);
        va_end(args);
    }
    void Logger::LogDebug(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, Severity::Debug, format, args);
        va_end(args);
    }

    void Logger::LogWarning(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, Severity::Warning, format, args);
        va_end(args);
    }

    void Logger::LogError(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, Severity::Error, format, args);
        va_end(args);
    }

    void Logger::LogCritical(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        Logger::Log(mName, Severity::Critical, format, args);
        va_end(args);
    }

    void Logger::Init()
    {
        if (std::filesystem::exists(Logger::sCurrentFile))
        {
            if (std::filesystem::exists(Logger::sPreviousFile))
                std::filesystem::remove(Logger::sPreviousFile);

            if (rename(Logger::sCurrentFile, Logger::sPreviousFile))
                Logger("Logger").Log(Severity::Debug, "Failed to rename log file %s to %s", Logger::sCurrentFile, Logger::sPreviousFile);
        }
    }

    void Logger::Shutdown()
    {
        Flush();
    }

    void Logger::Flush()
    {
        if (!Logger::sLogToFile)
            return;

        std::filesystem::path filepath{ Logger::sCurrentFile };
        std::filesystem::create_directories(filepath.parent_path());

        FILE* file = fopen(Logger::sCurrentFile, "a");
        if (file)
        {
            for (auto message : Logger::sBuffer)
                fwrite(message.c_str(), sizeof(char), message.length(), file);
            fclose(file);
            Logger::sBuffer.clear();
        }
        else
            Logger::sLogToFile = false;
    }

    Uint Logger::GetSeverityMaxBufferCount(Severity severity)
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

    const char* Logger::GetSeverityID(Severity severity)
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

    const char* Logger::GetSeverityConsoleColor(Severity severity)
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

    void Logger::Log(const char* name, Severity severity, const char* format, va_list args)
    {
        Uint length = vsnprintf(nullptr, 0, format, args) + 1;
        char* buf = new char[length];
        vsnprintf(buf, length, format, args);

        String message(buf);
        delete[] buf;

        std::vector<String> messages;

        Uint lastIndex = 0;
        for (Uint i = 0; i < message.length(); i++)
        {
            if (message[i] == '\n')
            {
                messages.push_back(message.substr(lastIndex, i - lastIndex));
                lastIndex = i + 1;
            }
            else if (i == message.length() - 1)
                messages.push_back(message.substr(lastIndex));
        }

        for (String msg : messages)
        {
            String logMsg = "";
            String systemConsoleMsg = "";
            String editorConsoleMsg = "";

            constexpr Uint timeBufferSize  = 16;
            std::time_t    currentTime      = std::time(nullptr);
            char           timeBuffer[timeBufferSize];

            if (Logger::sLogToFile)
                logMsg += "[" + String(name) + "]";
            if (Logger::sLogToConsole)
                systemConsoleMsg += String(Logger::GetSeverityConsoleColor(severity)) + "[" + String(name) + "]";

            if (std::strftime(timeBuffer, timeBufferSize, "[%H:%M:%S]", std::localtime(&currentTime)))
            {
                if (Logger::sLogToFile)
                    logMsg += timeBuffer;
                if (Logger::sLogToConsole)
                    systemConsoleMsg += timeBuffer;
                if (Logger::sLogToEditorConsole)
                    editorConsoleMsg += timeBuffer;
            }

            if (Logger::sLogToFile)
                logMsg += " " + String(Logger::GetSeverityID(severity)) + ": " + msg + "\n";
            if (Logger::sLogToConsole)
                systemConsoleMsg += " " + String(Logger::GetSeverityID(severity)) + ": " + msg + +"\033[0m " + "\n";

            if (Logger::sLogToFile)
                Logger::sBuffer.push_back(logMsg);
            if (Logger::sLogToConsole)
                printf("%s", systemConsoleMsg.c_str());

            //ImGui Console
            if (Logger::sLogToEditorConsole)
                editorConsoleMsg += " " + String(Logger::GetSeverityID(severity)) + ": " + msg;
            if (Logger::sLogToEditorConsole)
                Console::Get()->Submit(editorConsoleMsg, severity);
        }

        if (Logger::sLogToFile)
            if (Logger::sBuffer.size() > Logger::GetSeverityMaxBufferCount(severity))
                Flush();
    }
}
