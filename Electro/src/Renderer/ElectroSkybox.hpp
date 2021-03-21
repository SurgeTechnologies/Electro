//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Renderer/ElectroPipeline.hpp"
#include "Renderer/ElectroTexture.hpp"
#include "Renderer/ElectroConstantBuffer.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    class Skybox : public IElectroRef
    {
    public:
        Skybox() = default;
        Skybox(const Ref<TextureCube>& texture);
        ~Skybox() = default;

        void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        void SetCubemapTexture(const Ref<TextureCube>& texture) { mTexture = texture; }

        /*
        Get some cool skyboxes from http://www.humus.name/index.php?page=Textures
        Be sure to rename the image files as Spike Engine needs them to be,
        For more information on the Spike-Skybox naming see the TextureCube class in Texture.h file
        */
        static Ref<Skybox> Create(const Ref<TextureCube>& texture);
    private:
        Ref<ConstantBuffer> mSkyboxCBuffer;
        Ref<Pipeline> mPipeline;
        Ref<TextureCube> mTexture;
    };
}