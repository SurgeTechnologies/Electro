//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "ElectroPhysicsLayer.hpp"

namespace Electro
{
    Vector<PhysicsLayer> PhysicsLayerManager::sLayers;
    PhysicsLayer PhysicsLayerManager::sNullLayer = { 0, "NULL", 0, -1 };

    template<typename T, typename ConditionFunction>
    static bool RemoveIfExists(Vector<T>& vector, ConditionFunction condition)
    {
        for (Vector<T>::iterator it = vector.begin(); it != vector.end(); ++it)
        {
            if (condition(*it))
            {
                vector.erase(it);
                return true;
            }
        }

        return false;
    }

    Uint PhysicsLayerManager::AddLayer(const String& name, bool setCollisions)
    {
        Uint layerId = GetNextLayerID();
        PhysicsLayer layer = { layerId, name, BIT(layerId), BIT(layerId) };
        sLayers.insert(sLayers.begin() + layerId, layer);

        if (setCollisions)
        {
            for (const auto& layer2 : sLayers)
            {
                SetLayerCollision(layer.LayerID, layer2.LayerID, true);
            }
        }

        return layer.LayerID;
    }

    void PhysicsLayerManager::RemoveLayer(Uint layerId)
    {
        PhysicsLayer& layerInfo = GetLayer(layerId);

        for (auto& otherLayer : sLayers)
        {
            if (otherLayer.LayerID == layerId)
                continue;

            if (otherLayer.CollidesWith & layerInfo.BitValue)
            {
                otherLayer.CollidesWith &= ~layerInfo.BitValue;
            }
        }

        RemoveIfExists<PhysicsLayer>(sLayers, [&](const PhysicsLayer& layer) { return layer.LayerID == layerId; });
    }

    void PhysicsLayerManager::SetLayerCollision(Uint layerId, Uint otherLayer, bool shouldCollide)
    {
        if (ShouldCollide(layerId, otherLayer) && shouldCollide)
            return;

        PhysicsLayer& layerInfo = GetLayer(layerId);
        PhysicsLayer& otherLayerInfo = GetLayer(otherLayer);

        if (shouldCollide)
        {
            layerInfo.CollidesWith |= otherLayerInfo.BitValue;
            otherLayerInfo.CollidesWith |= layerInfo.BitValue;
        }
        else
        {
            layerInfo.CollidesWith &= ~otherLayerInfo.BitValue;
            otherLayerInfo.CollidesWith &= ~layerInfo.BitValue;
        }
    }

    Vector<PhysicsLayer> PhysicsLayerManager::GetLayerCollisions(Uint layerId)
    {
        const PhysicsLayer& layer = GetLayer(layerId);

        Vector<PhysicsLayer> layers;
        for (const auto& otherLayer : sLayers)
        {
            if (otherLayer.LayerID == layerId)
                continue;

            if (layer.CollidesWith & otherLayer.BitValue)
                layers.push_back(otherLayer);
        }

        return layers;
    }

    PhysicsLayer& PhysicsLayerManager::GetLayer(Uint layerId)
    {
        return layerId >= sLayers.size() ? sNullLayer : sLayers[layerId];
    }

    PhysicsLayer& PhysicsLayerManager::GetLayer(const String& layerName)
    {
        for (auto& layer : sLayers)
        {
            if (layer.Name == layerName)
                return layer;
        }

        return sNullLayer;
    }

    bool PhysicsLayerManager::ShouldCollide(Uint layer1, Uint layer2)
    {
        return GetLayer(layer1).CollidesWith & GetLayer(layer2).BitValue;
    }

    bool PhysicsLayerManager::IsLayerValid(Uint layerId)
    {
        const PhysicsLayer& layer = GetLayer(layerId);
        return layer.LayerID != sNullLayer.LayerID && layer.IsValid();
    }

    Uint PhysicsLayerManager::GetNextLayerID()
    {
        int32_t lastId = -1;

        for (const auto& layer : sLayers)
        {
            if (lastId != -1)
            {
                if (layer.LayerID != lastId + 1)
                {
                    return lastId + 1;
                }
            }

            lastId = layer.LayerID;
        }

        return sLayers.size();
    }
}
