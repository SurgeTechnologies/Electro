//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    enum class Severity
    {
        Trace,
        Info,
        Debug,
        Warning,
        Error,
        Critical
    };

    class Logger
    {
    public:
        Logger(const char* name);

        void Log(Severity severity, const char* format, ...);
        void LogTrace(const char* format, ...);
        void LogInfo(const char* format, ...);
        void LogDebug(const char* format, ...);
        void LogWarning(const char* format, ...);
        void LogError(const char* format, ...);
        void LogCritical(const char* format, ...);
        inline static Logger GetCoreLogger() { return sCoreLogger; };

        void SetLogToFile(bool value) { sLogToFile = value; }
        void SetLogToSystemConsole(bool value) { sLogToConsole = value; }
    public:
        static void Init();
        static void Shutdown();
        static void Flush();

    private:
        static Uint GetSeverityMaxBufferCount(Severity severity);
        static const char* GetSeverityID(Severity severity);
        static const char* GetSeverityConsoleColor(Severity severity);
        static void Log(const char* name, Severity severity, const char* format, va_list args);

    private:
        const char* mName;
    private:
        static Logger sCoreLogger;
        static Vector<String> sBuffer;

        static bool        sLogToFile;
        static bool        sLogToConsole;
        static bool        sLogToEditorConsole;
        static const char* sPreviousFile;
        static const char* sCurrentFile;
    };
}

/* Formatters
*   %c                  Character
    %d                  Signed integer
    %e or %E            Scientific notation of floats
    %f                  Float values
    %g or %G            Similar as %e or %E
    %hi                 Signed integer (short)
    %hu                 Unsigned Integer (short)
    %i                  Unsigned integer
    %l or %ld or %li    Long
    %lf                 Double
    %Lf                 Long double
    %lu                 Unsigned int or unsigned long
    %lli or %lld        Long long
    %llu                Unsigned long long
    %o                  Octal representation
    %p                  Pointer
    %s                  String
    %u                  Unsigned int
    %x or %X            Hexadecimal representation
    %n                  Prints nothing
    %%                  Prints % character
 **/

#define ELECTRO_TRACE(...)    ::Electro::Logger::GetCoreLogger().LogTrace(__VA_ARGS__)
#define ELECTRO_DEBUG(...)    ::Electro::Logger::GetCoreLogger().LogDebug(__VA_ARGS__)
#define ELECTRO_INFO(...)     ::Electro::Logger::GetCoreLogger().LogInfo(__VA_ARGS__)
#define ELECTRO_WARN(...)     ::Electro::Logger::GetCoreLogger().LogWarning(__VA_ARGS__)
#define ELECTRO_ERROR(...)    ::Electro::Logger::GetCoreLogger().LogError(__VA_ARGS__)
#define ELECTRO_CRITICAL(...) ::Electro::Logger::GetCoreLogger().LogCritical(__VA_ARGS__)
