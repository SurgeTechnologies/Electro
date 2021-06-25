//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "PhysXInternal.hpp"
#include "PhysicsActor.hpp"
#include "PhysXUtils.hpp"
#include "PhysicsMeshSerializer.hpp"
#include "Core/Timer.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Math/Math.hpp"

namespace Electro
{
    static PhysicsErrorCallback sErrorCallback;
    static PhysicsAssertHandler sAssertHandler;
    static physx::PxDefaultAllocator sAllocatorCallback;
    static physx::PxDefaultCpuDispatcher* sDefaultCpuDispatcher;
    static physx::PxFoundation* sFoundation;
    static physx::PxPvd* sPVD;
    static physx::PxPhysics* sPhysics;
    static physx::PxCooking* sCookingFactory;
    static ContactListener sContactListener;

    void PhysXInternal::Init()
    {
        E_ASSERT(!sFoundation, "Already initialized internal PhysX!");

        // Setup the foundation
        sFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, sAllocatorCallback, sErrorCallback);
        E_ASSERT(sFoundation, "Cannot create PhysX foundation!");

        // Create a PDV instance
        sPVD = PxCreatePvd(*sFoundation);
        if (sPVD)
        {
            physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 0001, 10);
            sPVD->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
        }

        // Create an instance of the PhysX physics SDK
        physx::PxTolerancesScale scale = physx::PxTolerancesScale();
        scale.length = 10;
        sPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *sFoundation, scale, true, sPVD);
        E_ASSERT(sPhysics, "PhysX Physics creation failed!");

        // Create the cooking factory
        sCookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *sFoundation, sPhysics->getTolerancesScale());
        E_ASSERT(sCookingFactory, "PhysX Cooking creation Failed!");
        sDefaultCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
        PxSetAssertHandler(sAssertHandler);
        Log::Info("Initialized PhysX");
    }

    void PhysXInternal::Shutdown()
    {
        EPX_RELEASE(sDefaultCpuDispatcher);
        EPX_RELEASE(sCookingFactory);
        EPX_RELEASE(sPhysics);
        EPX_RELEASE(sFoundation);
    }

    void PhysXInternal::AddBoxCollider(PhysicsActor& actor)
    {
        BoxColliderComponent& collider = actor.mEntity.GetComponent<BoxColliderComponent>();
        glm::vec3 colliderSize = collider.Size;
        glm::vec3 size = actor.mEntity.Transform().Scale;

        if (size.x != 0.0f) colliderSize.x *= size.x;
        if (size.y != 0.0f) colliderSize.y *= size.y;
        if (size.z != 0.0f) colliderSize.z *= size.z;

        physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(colliderSize.x / 2.0f, colliderSize.y / 2.0f, colliderSize.z / 2.0f);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor.mInternalActor, boxGeometry, *actor.mInternalMaterial);

        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(PhysXUtils::ToPhysXTransform(glm::translate(glm::mat4(1.0f), collider.Offset)));
    }

    void PhysXInternal::AddSphereCollider(PhysicsActor& actor)
    {
        SphereColliderComponent& collider = actor.mEntity.GetComponent<SphereColliderComponent>();
        float colliderRadius = collider.Radius;
        glm::vec3 size = actor.mEntity.Transform().Scale;

        if (size.x != 0.0f) colliderRadius *= size.x;

        physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(colliderRadius);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor.mInternalActor, sphereGeometry, *actor.mInternalMaterial);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
    }

    void PhysXInternal::AddCapsuleCollider(PhysicsActor& actor)
    {
        auto& collider = actor.mEntity.GetComponent<CapsuleColliderComponent>();

        float colliderRadius = collider.Radius;
        float colliderHeight = collider.Height;
        glm::vec3 size = actor.mEntity.Transform().Scale;
        if (size.x != 0.0f)
            colliderRadius *= (size.x / 2.0f);
        if (size.y != 0.0f)
            colliderHeight *= size.y;

        physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(colliderRadius, colliderHeight / 2.0f);
        physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor.mInternalActor, capsuleGeometry, *actor.mInternalMaterial);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
        shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
    }

    void PhysXInternal::AddMeshCollider(PhysicsActor& actor)
    {
        MeshColliderComponent& collider = actor.mEntity.GetComponent<MeshColliderComponent>();
        glm::vec3 size = actor.mEntity.Transform().Scale;

        if (collider.IsConvex)
        {
            Vector<physx::PxShape*> shapes = CreateConvexMesh(collider, size);
            for (physx::PxShape* shape : shapes)
            {
                physx::PxMaterial* materials[] = { actor.mInternalMaterial };
                shape->setMaterials(materials, 1);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
                bool status = actor.mInternalActor->attachShape(*shape);
                shape->release();
                if (!status)
                    shape = nullptr;
            }
        }
        else // Creates a Triangle Mesh
        {
            Vector<physx::PxShape*> shapes = CreateTriangleMesh(collider, size);
            for (physx::PxShape* shape : shapes)
            {
                physx::PxMaterial* materials[] = { actor.mInternalMaterial };
                shape->setMaterials(materials, 1);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger);
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger);
                bool status = actor.mInternalActor->attachShape(*shape);
                shape->release();
                if (!status)
                    shape = nullptr;
            }
        }
    }

    bool PhysXInternal::Raycast(RaycastHit* hit, const glm::vec3& origin, const glm::vec3& direction, float maxDistance)
    {
        physx::PxScene* scene = static_cast<physx::PxScene*>(PhysicsEngine::GetPhysicsScene());
        physx::PxRaycastBuffer hitResult;
        bool status = scene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(direction), maxDistance, hitResult);
        if (status)
        {
            Entity& entity = *(Entity*)hitResult.block.actor->userData;
            auto tag = entity.GetComponent<TagComponent>().Tag;
            hit->EntityUUID = entity.GetUUID();
            hit->Distance = hitResult.block.distance;
            hit->Position = PhysXUtils::FromPhysXVector(hitResult.block.position);
            hit->Normal = PhysXUtils::FromPhysXVector(hitResult.block.normal);
        }
        return status;
    }

    Vector<physx::PxShape*> PhysXInternal::CreateConvexMesh(MeshColliderComponent& collider, const glm::vec3& size)
    {
        Vector<physx::PxShape*> shapes;

        const physx::PxCookingParams& currentParams = sCookingFactory->getParams();
        physx::PxCookingParams newParams = currentParams;
        newParams.planeTolerance = 0.0f;
        newParams.meshPreprocessParams = physx::PxMeshPreprocessingFlags(physx::PxMeshPreprocessingFlag::eWELD_VERTICES);
        newParams.meshWeldTolerance = 0.01f;
        sCookingFactory->setParams(newParams);

        const Vector<Vertex>& vertices = collider.CollisionMesh->GetVertices();
        const Vector<Index>& indices = collider.CollisionMesh->GetIndices();
        const Vector<Submesh>& submeshes = collider.CollisionMesh->GetSubmeshes();
        bool isConvex = true;
        bool generateDebugGeometry = false;

        E_ASSERT(submeshes.size() != 0, "Invalid Mesh!");

        if (!PhysicsMeshSerializer::Exists(submeshes[0].MeshName, isConvex))
        {
            for (const Submesh& submesh : collider.CollisionMesh->GetSubmeshes())
            {
                PhysicsMeshSerializer::DeleteIfExists(submesh.MeshName, isConvex);

                // https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Geometry.html#convex-meshes
                physx::PxConvexMeshDesc convexDesc;

                // Vertices
                convexDesc.points.count = submesh.VertexCount;
                convexDesc.points.stride = sizeof(Vertex);
                convexDesc.points.data = &vertices[submesh.BaseVertex];

                // Indices
                convexDesc.indices.count = submesh.IndexCount / 3;
                convexDesc.indices.data = &indices[submesh.BaseIndex / 3];
                convexDesc.indices.stride = sizeof(Index);
                convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eSHIFT_VERTICES;

                // Cooking the mesh using stream serialization
                physx::PxDefaultMemoryOutputStream cookedResult;
                physx::PxConvexMeshCookingResult::Enum result;

                Timer cookTime;
                if (!sCookingFactory->cookConvexMesh(convexDesc, cookedResult, &result))
                {
                    Log::Error("[PhysicsEngine] Failed to cook convex mesh {0}", submesh.MeshName);
                    continue;
                }
                Log::Trace("[PhysicsEngine] Convex Mesh(Submesh) named {0} took {1} seconds to cook!", submesh.MeshName, cookTime.Elapsed());

                physx::PxDefaultMemoryInputData input(cookedResult.getData(), cookedResult.getSize());
                physx::PxConvexMesh* physicsMesh = sPhysics->createConvexMesh(input);
                physx::PxConvexMeshGeometry convexGeometry = physx::PxConvexMeshGeometry(physicsMesh, physx::PxMeshScale(PhysXUtils::ToPhysXVector(size)));
                convexGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

                if (physicsMesh)
                    PhysicsMeshSerializer::Serialize(cookedResult, submesh.MeshName, isConvex);

                // Dummy material, replaced at runtime
                physx::PxMaterial* material = sPhysics->createMaterial(0, 0, 0);

                physx::PxShape* shape = sPhysics->createShape(convexGeometry, *material, true);
                shape->setLocalPose(PhysXUtils::ToPhysXTransform(submesh.Transform));
                shapes.push_back(shape);
                EPX_RELEASE(material);
                EPX_RELEASE(physicsMesh);
            }
        }
        else
        {
            for (const Submesh& submesh : submeshes)
            {
                Buffer cookedResult = PhysicsMeshSerializer::Deserialize(submesh.MeshName, isConvex);

                physx::PxDefaultMemoryInputData input(cookedResult.As<physx::PxU8>(), cookedResult.GetSize());
                physx::PxConvexMesh* physicsMesh = sPhysics->createConvexMesh(input);
                physx::PxConvexMeshGeometry convexGeometry = physx::PxConvexMeshGeometry(physicsMesh, physx::PxMeshScale(PhysXUtils::ToPhysXVector(size)));
                convexGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

                // Dummy material, replaced at runtime
                physx::PxMaterial* material = sPhysics->createMaterial(0, 0, 0);

                physx::PxShape* shape = sPhysics->createShape(convexGeometry, *material, true);
                shape->setLocalPose(PhysXUtils::ToPhysXTransform(submesh.Transform));
                shapes.push_back(shape);
                EPX_RELEASE(material);
                EPX_RELEASE(physicsMesh);
                cookedResult.Release();
            }
        }

        // Generates the debug draw outline
        if (collider.ProcessedMeshes.empty())
        {
            for (physx::PxShape* shape : shapes)
            {
                physx::PxConvexMeshGeometry convexGeometry;
                shape->getConvexMeshGeometry(convexGeometry);
                physx::PxConvexMesh* mesh = convexGeometry.convexMesh;

                // https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/ThirdParty/PhysX3/NvCloth/samples/SampleBase/renderer/ConvexRenderMesh.cpp
                const Uint nbPolygons = mesh->getNbPolygons();
                const physx::PxVec3* convexVertices = mesh->getVertices();
                const physx::PxU8* convexIndices = mesh->getIndexBuffer();

                Uint nbVertices = 0;
                Uint nbFaces = 0;

                Vector<Vertex> collisionVertices;
                Vector<Index> collisionIndices;

                Uint vertCounter = 0;
                Uint indexCounter = 0;

                for (Uint i = 0; i < nbPolygons; i++)
                {
                    physx::PxHullPolygon polygon;
                    mesh->getPolygonData(i, polygon);
                    nbVertices += polygon.mNbVerts;
                    nbFaces += (polygon.mNbVerts - 2) * 3;

                    Uint vI0 = vertCounter;

                    for (Uint vI = 0; vI < polygon.mNbVerts; vI++)
                    {
                        Vertex v;
                        v.Position = PhysXUtils::FromPhysXVector(convexVertices[convexIndices[polygon.mIndexBase + vI]]);
                        collisionVertices.push_back(v);
                        vertCounter++;
                    }

                    for (Uint vI = 1; vI < Uint(polygon.mNbVerts) - 1; vI++)
                    {
                        Index index;
                        index.V1 = Uint(vI0);
                        index.V2 = Uint(vI0 + vI + 1);
                        index.V3 = Uint(vI0 + vI);
                        collisionIndices.push_back(index);
                        indexCounter++;
                    }

                    collider.ProcessedMeshes.emplace_back(Ref<Mesh>::Create(collisionVertices, collisionIndices, PhysXUtils::FromPhysXTransform(shape->getLocalPose())));
                }
            }
        }
        sCookingFactory->setParams(currentParams);
        return shapes;
    }

    Vector<physx::PxShape*> PhysXInternal::CreateTriangleMesh(MeshColliderComponent& collider, const glm::vec3& scale)
    {
        Vector<physx::PxShape*> shapes;

        const Vector<Vertex>& vertices = collider.CollisionMesh->GetVertices();
        const Vector<Index>& indices = collider.CollisionMesh->GetIndices();
        const Vector<Submesh>& submeshes = collider.CollisionMesh->GetSubmeshes();
        bool isConvex = false;

        E_ASSERT(submeshes.size() != 0, "Invalid Mesh!");

        if (!PhysicsMeshSerializer::Exists(submeshes[0].MeshName, isConvex))
        {
            for (const Submesh& submesh : submeshes)
            {
                PhysicsMeshSerializer::DeleteIfExists(submesh.MeshName, false);

                // https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Geometry.html#triangle-meshes
                physx::PxTriangleMeshDesc triangleDesc;
                triangleDesc.points.count = submesh.VertexCount;
                triangleDesc.points.stride = sizeof(Vertex);
                triangleDesc.points.data = &vertices[submesh.BaseVertex];
                triangleDesc.triangles.count = submesh.IndexCount / 3;
                triangleDesc.triangles.data = &indices[submesh.BaseIndex / 3];
                triangleDesc.triangles.stride = sizeof(Index);

                physx::PxDefaultMemoryOutputStream cookedResult;
                physx::PxTriangleMeshCookingResult::Enum result;

                Timer cookTime;
                if (!sCookingFactory->cookTriangleMesh(triangleDesc, cookedResult, &result))
                {
                    Log::Error("[PhysicsEngine] Failed to cook triangle mesh: {0}", submesh.MeshName);
                    continue;
                }
                Log::Trace("[PhysicsEngine] Triangle Mesh named {0} took {1} seconds to cook!", collider.CollisionMesh->GetName(), cookTime.Elapsed());

                glm::vec3 submeshTranslation, submeshRotation, submeshScale;
                Math::DecomposeTransform(submesh.LocalTransform, submeshTranslation, submeshRotation, submeshScale);

                physx::PxDefaultMemoryInputData input(cookedResult.getData(), cookedResult.getSize());
                physx::PxTriangleMesh* physicsMesh = sPhysics->createTriangleMesh(input);
                physx::PxTriangleMeshGeometry triangleGeometry = physx::PxTriangleMeshGeometry(physicsMesh, physx::PxMeshScale(PhysXUtils::ToPhysXVector(submeshScale * scale)));

                if (physicsMesh)
                    PhysicsMeshSerializer::Serialize(cookedResult, submesh.MeshName, isConvex);

                // Dummy material, replaced at runtime
                physx::PxMaterial* material = sPhysics->createMaterial(0, 0, 0);

                physx::PxShape* shape = sPhysics->createShape(triangleGeometry, *material, true);
                shape->setLocalPose(PhysXUtils::ToPhysXTransform(submeshTranslation, submeshRotation));
                shapes.push_back(shape);

                EPX_RELEASE(material);
                EPX_RELEASE(physicsMesh);
            }
        }
        else
        {
            for (const Submesh& submesh : submeshes)
            {
                glm::vec3 submeshTranslation, submeshRotation, submeshScale;
                Math::DecomposeTransform(submesh.LocalTransform, submeshTranslation, submeshRotation, submeshScale);

                Buffer cookedResult = PhysicsMeshSerializer::Deserialize(submesh.MeshName, isConvex);

                physx::PxDefaultMemoryInputData input(cookedResult.As<physx::PxU8>(), cookedResult.GetSize());
                physx::PxTriangleMesh* physicsMesh = sPhysics->createTriangleMesh(input);
                physx::PxTriangleMeshGeometry triangleGeometry = physx::PxTriangleMeshGeometry(physicsMesh, physx::PxMeshScale(PhysXUtils::ToPhysXVector(submeshScale * scale)));

                // Dummy material, replaced at runtime
                physx::PxMaterial* material = sPhysics->createMaterial(0, 0, 0);

                physx::PxShape* shape = sPhysics->createShape(triangleGeometry, *material, true);
                shape->setLocalPose(PhysXUtils::ToPhysXTransform(submeshTranslation, submeshRotation));
                shapes.push_back(shape);

                EPX_RELEASE(material);
                EPX_RELEASE(physicsMesh);
                cookedResult.Release();
            }
        }

        // Generates the debug draw outline
        if (collider.ProcessedMeshes.empty())
        {
            for (physx::PxShape* shape : shapes)
            {
                physx::PxTriangleMeshGeometry triangleGeometry;
                shape->getTriangleMeshGeometry(triangleGeometry);
                physx::PxTriangleMesh* mesh = triangleGeometry.triangleMesh;

                const Uint nbVerts = mesh->getNbVertices();
                const physx::PxVec3* triangleVertices = mesh->getVertices();

                const Uint nbTriangles = mesh->getNbTriangles();
                const physx::PxU16* tris = (const physx::PxU16*)mesh->getTriangles();

                Vector<Vertex> vertices;
                Vector<Index> indices;

                for (Uint v = 0; v < nbVerts; v++)
                {
                    Vertex v1;
                    v1.Position = PhysXUtils::FromPhysXVector(triangleVertices[v]);
                    vertices.push_back(v1);
                }

                for (Uint tri = 0; tri < nbTriangles; tri++)
                {
                    Index index;
                    index.V1 = tris[3 * tri + 0];
                    index.V2 = tris[3 * tri + 1];
                    index.V3 = tris[3 * tri + 2];
                    indices.push_back(index);
                }

                glm::mat4 scale = glm::scale(glm::mat4(1.0f), PhysXUtils::FromPhysXVector(triangleGeometry.scale.scale));
                glm::mat4 transform = PhysXUtils::FromPhysXTransform(shape->getLocalPose()) * scale;
                collider.ProcessedMeshes.push_back(Ref<Mesh>::Create(vertices, indices, transform));
            }
        }
        return shapes;
    }

    static physx::PxBroadPhaseType::Enum ElectroToPhysXBroadphaseType(BroadphaseType type)
    {
        switch (type)
        {
            case BroadphaseType::SweepAndPrune:     return physx::PxBroadPhaseType::eSAP;
            case BroadphaseType::MultiBoxPrune:     return physx::PxBroadPhaseType::eMBP;
            case BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
        }

        return physx::PxBroadPhaseType::eABP;
    }

    static physx::PxFrictionType::Enum ElectroToPhysXFrictionType(FrictionType type)
    {
        switch (type)
        {
            case FrictionType::Patch:           return physx::PxFrictionType::ePATCH;
            case FrictionType::OneDirectional:  return physx::PxFrictionType::eONE_DIRECTIONAL;
            case FrictionType::TwoDirectional:  return physx::PxFrictionType::eTWO_DIRECTIONAL;
        }

        return physx::PxFrictionType::ePATCH;
    }

    physx::PxScene* PhysXInternal::CreateScene()
    {
        physx::PxSceneDesc sceneDesc(sPhysics->getTolerancesScale());
        const PhysicsSettings& settings = PhysicsEngine::GetSettings();
        sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
        sceneDesc.broadPhaseType = ElectroToPhysXBroadphaseType(settings.BroadphaseAlgorithm);
        sceneDesc.cpuDispatcher = sDefaultCpuDispatcher;
        sceneDesc.filterShader = PhysXUtils::ElectroCollisionFilterShader;
        sceneDesc.simulationEventCallback = &sContactListener;
        sceneDesc.frictionType = ElectroToPhysXFrictionType(settings.FrictionModel);
        sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD; // Enable continious collision detection

        E_ASSERT(sceneDesc.isValid(), "Scene is not valid!");
        return sPhysics->createScene(sceneDesc);
    }

    physx::PxPhysics& PhysXInternal::GetPhysics()
    {
        return *sPhysics;
    }

    physx::PxAllocatorCallback& PhysXInternal::GetAllocator()
    {
        return sAllocatorCallback;
    }

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
                Log::Critical("[PhysX]: {0}: {1}", errorMessage, message); E_INTERNAL_ASSERT("PhysX Terminated..."); break;
        }
    }

    void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool& ignore)
    {
        Log::Critical("[PhysX Error]: {0}:{1} - {2}", file, line, exp);
    }

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

    void ContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
    {
        Entity& a = *(Entity*)pairHeader.actors[0]->userData;
        Entity& b = *(Entity*)pairHeader.actors[1]->userData;

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
        Entity& a = *(Entity*)pairs->triggerActor->userData;
        Entity& b = *(Entity*)pairs->otherActor->userData;

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
