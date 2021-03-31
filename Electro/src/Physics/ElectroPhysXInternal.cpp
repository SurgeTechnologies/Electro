//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysXInternal.hpp"

namespace Electro
{
    static PhysicsErrorCallback sErrorCallback;
    static PhysicsAssertHandler sAssertHandler;
    static physx::PxDefaultAllocator sAllocatorCallback;
    static physx::PxFoundation* sFoundation;
    static physx::PxPvd* sPVD;
    static physx::PxPhysics* sPhysics;
    static physx::PxCooking* sCookingFactory;

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
                ELECTRO_INFO("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); break;
            case physx::PxErrorCode::eDEBUG_WARNING:
            case physx::PxErrorCode::ePERF_WARNING:
                ELECTRO_WARN("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); break;
            case physx::PxErrorCode::eINVALID_PARAMETER:
            case physx::PxErrorCode::eINVALID_OPERATION:
            case physx::PxErrorCode::eOUT_OF_MEMORY:
            case physx::PxErrorCode::eINTERNAL_ERROR:
                ELECTRO_ERROR("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); break;
            case physx::PxErrorCode::eABORT:
            case physx::PxErrorCode::eMASK_ALL:
                ELECTRO_CRITICAL("[PhysX]: %s: %s at %s (line: %d)", errorMessage, message, file, line); E_INTERNAL_ASSERT("PhysX Terminated..."); break;
        }
    }

    void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool& ignore)
    {
        ELECTRO_CRITICAL("[PhysX Error]: %s:%d - %s", file, line, exp);
    }

    void PhysXInternal::Init()
    {
        E_ASSERT(!sFoundation, "Already initialized internal PhysX!");

        //Setup the foundation
        sFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, sAllocatorCallback, sErrorCallback);
        E_ASSERT(sFoundation, "Cannot create PhysX foundation!");

        //Create a PDV instance
        sPVD = PxCreatePvd(*sFoundation);
        if (sPVD)
        {
            physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
            sPVD->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
        }

        //Create an instance of the PhysX physics SDK
        physx::PxTolerancesScale scale = physx::PxTolerancesScale();
        scale.length = 10;
        sPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *sFoundation, scale, true, sPVD);
        E_ASSERT(sPhysics, "PhysX Physics creation failed!");

        //Create the cooking factory for bulk serialization
        sCookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *sFoundation, sPhysics->getTolerancesScale());
        E_ASSERT(sCookingFactory, "PhysX Cooking creation Failed!");

        PxSetAssertHandler(sAssertHandler);
        ELECTRO_INFO("Initialized PhysX");
    }

    void PhysXInternal::ShutDown()
    {
        sCookingFactory->release();
        sPhysics->release();
        sFoundation->release(); //Remember to release() it at the end!
    }
}