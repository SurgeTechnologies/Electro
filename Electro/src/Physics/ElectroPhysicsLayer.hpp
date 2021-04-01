//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    struct PhysicsLayer
    {
        uint32_t LayerID;
        String Name;
        uint32_t BitValue;
        int32_t CollidesWith = 0;
        bool IsValid() const { return LayerID >= 0 && !Name.empty() && BitValue > 0; }
    };

    class PhysicsLayerManager
    {
    public:
        static uint32_t AddLayer(const String& name, bool setCollisions = true);
        static void RemoveLayer(uint32_t layerId);

        static void SetLayerCollision(uint32_t layerId, uint32_t otherLayer, bool shouldCollide);
        static Vector<PhysicsLayer> GetLayerCollisions(uint32_t layerId);

        static const Vector<PhysicsLayer>& GetLayers() { return sLayers; }

        static PhysicsLayer& GetLayer(uint32_t layerId);
        static PhysicsLayer& GetLayer(const String& layerName);
        static uint32_t GetLayerCount() { return sLayers.size(); }

        static bool ShouldCollide(uint32_t layer1, uint32_t layer2);
        static bool IsLayerValid(uint32_t layerId);
    private:
        static uint32_t GetNextLayerID();
    private:
        static Vector<PhysicsLayer> sLayers;
        static PhysicsLayer sNullLayer;
    };
}