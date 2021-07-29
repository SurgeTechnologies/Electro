//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Layer.hpp"
#include <vector>

namespace Electro
{
    class LayerManager
    {
    public:
        LayerManager() = default;
        ~LayerManager();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        Vector<Layer*>::iterator begin() { return mLayers.begin(); }
        Vector<Layer*>::iterator end() { return mLayers.end(); }
    private:
        Vector<Layer*> mLayers;
        Uint mLayerInsertIndex = 0;
    };
}
