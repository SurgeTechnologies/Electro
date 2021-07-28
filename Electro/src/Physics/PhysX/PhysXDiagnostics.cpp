//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysXDiagnostics.hpp"

namespace Electro
{
    void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
    {
        const char* errorMessage = NULL;

        switch (code)
        {
            case physx::PxErrorCode::eNO_ERROR:          errorMessage = "NO ERROR";           break;
            case physx::PxErrorCode::eDEBUG_INFO:        errorMessage = "INFO";               break;
            case physx::PxErrorCode::eDEBUG_WARNING:     errorMessage = "WARNING";            break;
            case physx::PxErrorCode::eINVALID_PARAMETER: errorMessage = "INVALID PARAMETER";  break;
            case physx::PxErrorCode::eINVALID_OPERATION: errorMessage = "INVALID OPERATION";  break;
            case physx::PxErrorCode::eOUT_OF_MEMORY:     errorMessage = "OUT OF MEMORY";      break;
            case physx::PxErrorCode::eINTERNAL_ERROR:    errorMessage = "INTERNAL ERROR";     break;
            case physx::PxErrorCode::eABORT:             errorMessage = "ABORT";              break;
            case physx::PxErrorCode::ePERF_WARNING:      errorMessage = "PERFOMANCE WARNING"; break;
            case physx::PxErrorCode::eMASK_ALL:          errorMessage = "UNKNOWN";            break;
        }

        switch (code)
        {
            case physx::PxErrorCode::eNO_ERROR:
            case physx::PxErrorCode::eDEBUG_INFO:
                Log::Info("[PhysX]: {0}: {1}", errorMessage, message); break;
            case physx::PxErrorCode::eDEBUG_WARNING:
            case physx::PxErrorCode::ePERF_WARNING:
                Log::Warn("[PhysX]: {0}: {1}", errorMessage, message); break;
            case physx::PxErrorCode::eINVALID_PARAMETER:
            case physx::PxErrorCode::eINVALID_OPERATION:
            case physx::PxErrorCode::eOUT_OF_MEMORY:
            case physx::PxErrorCode::eINTERNAL_ERROR:
                Log::Error("[PhysX]: {0}: {1}", errorMessage, message); break;
            case physx::PxErrorCode::eABORT:
            case physx::PxErrorCode::eMASK_ALL:
                Log::Critical("[PhysX]: {0}: {1}", errorMessage, message);
                E_INTERNAL_ASSERT("PhysX Terminated..."); break;
        }
    }

    void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool& ignore)
    {
    #ifdef E_DEBUG
        Log::Critical("[PhysX Error]: {0}:{1} - {2}", file, line, exp);
    #endif
    }
}