//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ContactListener.hpp"
#include "Scripting/ScriptEngine.hpp"
#include <PxRigidActor.h>

namespace Electro
{
    void ContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
    {
        PX_UNUSED(constraints);
        PX_UNUSED(count);
    }

    void ContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
    {
        for (Uint i = 0; i < count; i++)
        {
            physx::PxActor& actor = *actors[i];
            Entity& entity = *(Entity*)actor.userData;
            Log::Info("PhysX Actor Waking UP: UUID: {0}, Name: {1}", entity.GetUUID(), entity.GetComponent<TagComponent>().Tag);
        }
    }

    void ContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
    {
        for (Uint i = 0; i < count; i++)
        {
            physx::PxActor& actor = *actors[i];
            Entity& entity = *(Entity*)actor.userData;
            Log::Info("PhysX Actor going to Sleep: UUID: {0}, Name: {1}", entity.GetUUID(), entity.GetComponent<TagComponent>().Tag);
        }
    }

    // TODO: Make OnContact and OnTrigger pass the actual entity to which it collided
    void ContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
    {
        Entity& a = *static_cast<Entity*>(pairHeader.actors[0]->userData);
        Entity& b = *static_cast<Entity*>(pairHeader.actors[1]->userData);

        if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
        {
            if (ScriptEngine::IsEntityModuleValid(a))
                ScriptEngine::OnCollisionBegin(a);

            if (ScriptEngine::IsEntityModuleValid(b))
                ScriptEngine::OnCollisionBegin(b);
        }
        else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
        {
            if (ScriptEngine::IsEntityModuleValid(a))
                ScriptEngine::OnCollisionEnd(a);

            if (ScriptEngine::IsEntityModuleValid(b))
                ScriptEngine::OnCollisionEnd(b);
        }
    }

    void ContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
    {
        Entity& a = *static_cast<Entity*>(pairs->triggerActor->userData);
        Entity& b = *static_cast<Entity*>(pairs->otherActor->userData);

        if (pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            if (ScriptEngine::IsEntityModuleValid(a))
                ScriptEngine::OnTriggerBegin(a);

            if (ScriptEngine::IsEntityModuleValid(b))
                ScriptEngine::OnTriggerBegin(b);
        }
        else if (pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            if (ScriptEngine::IsEntityModuleValid(a))
                ScriptEngine::OnTriggerEnd(a);

            if (ScriptEngine::IsEntityModuleValid(b))
                ScriptEngine::OnTriggerEnd(b);
        }
    }

    void ContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
    {
        PX_UNUSED(bodyBuffer);
        PX_UNUSED(poseBuffer);
        PX_UNUSED(count);
    }
}