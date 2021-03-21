//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"
#include "ElectroLayer.hpp"
#include <vector>

namespace Electro
{
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack();

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