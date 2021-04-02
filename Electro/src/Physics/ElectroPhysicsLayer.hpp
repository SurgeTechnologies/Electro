//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    struct PhysicsLayer
    {
        Uint LayerID;
        String Name;
        Uint BitValue;
        int32_t CollidesWith = 0;
        bool IsValid() const { return LayerID >= 0 && !Name.empty() && BitValue > 0; }
    };

    class PhysicsLayerManager
    {
    public:
        static Uint AddLayer(const String& name, bool setCollisions = true);
        static void RemoveLayer(Uint layerId);

        static void SetLayerCollision(Uint layerId, Uint otherLayer, bool shouldCollide);
        static Vector<PhysicsLayer> GetLayerCollisions(Uint layerId);

        static const Vector<PhysicsLayer>& GetLayers() { return sLayers; }

        static PhysicsLayer& GetLayer(Uint layerId);
        static PhysicsLayer& GetLayer(const String& layerName);
        static Uint GetLayerCount() { return sLayers.size(); }

        static bool ShouldCollide(Uint layer1, Uint layer2);
        static bool IsLayerValid(Uint layerId);
    private:
        static Uint GetNextLayerID();
    private:
        static Vector<PhysicsLayer> sLayers;
        static PhysicsLayer sNullLayer;
    };
}