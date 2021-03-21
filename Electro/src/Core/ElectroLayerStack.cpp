//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroLayerStack.hpp"

namespace Electro
{
    LayerStack::~LayerStack()
    {
        for (Layer* layer : mLayers)
        {
            layer->OnDetach();
            delete layer;
        }
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        mLayers.emplace(mLayers.begin() + mLayerInsertIndex, layer);
        mLayerInsertIndex++;
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        mLayers.push_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        auto it = std::find(mLayers.begin(), mLayers.end(), layer);
        if (it != mLayers.end())
        {
            mLayers.erase(it);
            mLayerInsertIndex--;
            layer->OnDetach();
        }
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        auto it = std::find(mLayers.begin(), mLayers.end(), overlay);
        if (it != mLayers.end())
        {
            mLayers.erase(it);
            overlay->OnDetach();
        }
    }
}