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

    class ELogger
    {
    public:
        ELogger(const char* name);

        void ELog(Severity severity, const char* format, ...);
        void ELogTrace(const char* format, ...);
        void ELogInfo(const char* format, ...);
        void ELogDebug(const char* format, ...);
        void ELogWarning(const char* format, ...);
        void ELogError(const char* format, ...);
        void ELogCritical(const char* format, ...);
        inline static ELogger EGetCoreLogger() { return sCoreLogger; };

        void ESetLogToFile(bool value) { sLogToFile = value; }
        void ESetLogToSystemConsole(bool value) { sLogToConsole = value; }
    public:
        static void EInit();
        static void EShutdown();
        static void EFlush();

    private:
        static Uint EGetSeverityMaxBufferCount(Severity severity);
        static const char* EGetSeverityID(Severity severity);
        static const char* EGetSeverityConsoleColor(Severity severity);
        static void ELog(const char* name, Severity severity, const char* format, va_list args);

    private:
        const char* mName;
    private:
        static ELogger sCoreLogger;
        static Vector<String> sBuffer;

        static bool        sLogToFile;
        static bool        sLogToConsole;
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

#define ELECTRO_TRACE(...)    ::Electro::ELogger::EGetCoreLogger().ELogTrace(__VA_ARGS__)
#define ELECTRO_DEBUG(...)    ::Electro::ELogger::EGetCoreLogger().ELogDebug(__VA_ARGS__)
#define ELECTRO_INFO(...)     ::Electro::ELogger::EGetCoreLogger().ELogInfo(__VA_ARGS__)
#define ELECTRO_WARN(...)     ::Electro::ELogger::EGetCoreLogger().ELogWarning(__VA_ARGS__)
#define ELECTRO_ERROR(...)    ::Electro::ELogger::EGetCoreLogger().ELogError(__VA_ARGS__)
#define ELECTRO_CRITICAL(...) ::Electro::ELogger::EGetCoreLogger().ELogCritical(__VA_ARGS__)