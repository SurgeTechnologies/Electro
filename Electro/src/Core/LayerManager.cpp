//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "LayerManager.hpp"

namespace Electro
{
    LayerManager::~LayerManager()
    {
        for (Layer* layer : mLayers)
        {
            layer->Shutdown();
            delete layer;
        }
    }

    void LayerManager::PushLayer(Layer* layer)
    {
        mLayers.emplace(mLayers.begin() + mLayerInsertIndex, layer);
        mLayerInsertIndex++;
    }

    void LayerManager::PushOverlay(Layer* overlay)
    {
        mLayers.push_back(overlay);
    }

    void LayerManager::PopLayer(Layer* layer)
    {
        auto it = std::find(mLayers.begin(), mLayers.end(), layer);
        if (it != mLayers.end())
        {
            mLayers.erase(it);
            mLayerInsertIndex--;
            layer->Shutdown();
        }
    }

    void LayerManager::PopOverlay(Layer* overlay)
    {
        auto it = std::find(mLayers.begin(), mLayers.end(), overlay);
        if (it != mLayers.end())
        {
            mLayers.erase(it);
            overlay->Shutdown();
        }
    }
}
