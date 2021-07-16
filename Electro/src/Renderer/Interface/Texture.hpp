//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Asset/AssetBase.hpp"
#include "Renderer/Interface/Shader.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    struct Texture2DSpecification
    {
        Texture2DSpecification() = default;

        Texture2DSpecification(const String& path, bool generateMips, bool srgb)
            : Path(path), GenerateMips(generateMips), SRGB(srgb) {}

        Texture2DSpecification(const String& path)
            : Path(path) {}

        String Path = "";
        bool GenerateMips = false;
        bool SRGB = false;
    };

    class Texture2D : public Asset
    {
    public:
        virtual ~Texture2D() = default;

        virtual const Texture2DSpecification& GetSpecification() const = 0;

        // Binds the Texture to the pipeline
        virtual void Bind(Uint slot, const ShaderDomain shaderDomain) const = 0;

        // Unbinds the Texture from the pipeline
        virtual void Unbind(Uint slot, const ShaderDomain shaderDomain) const = 0;

        // Returns true if the Texture is HDR
        virtual bool IsHDR() const = 0;

        // Gets the Width of the loaded Texture
        virtual Uint GetWidth() const = 0;

        // Gets the Height of the loaded Texture
        virtual Uint GetHeight() const = 0;

        // Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        // Returns true if the texture is loaded to Electro perfectly
        virtual bool Loaded() = 0;

        static Ref<Texture2D> Create(const Texture2DSpecification& spec);
    };

    class Cubemap : public IElectroRef
    {
    public:
        virtual ~Cubemap() = default;

        // Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        // Returns the Path of the HDR texture
        virtual String GetPath() const = 0;

        // Returns the file name
        virtual String const GetName() const = 0;

        // Binds the Cubemap to the pipeline
        virtual void VSBind(Uint slot = 0) const = 0;
        virtual void PSBind(Uint slot = 0) const = 0;
        virtual void CSBind(Uint slot = 0) const = 0;

        // Binds the Cubemap from the pipeline
        virtual void Unbind(Uint slot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const = 0;

        // Generates the PreFilter map for the texture cube
        virtual RendererID GenIrradianceMap() = 0;

        // Generates the irradince map for the texture cube
        virtual RendererID GenPreFilter() = 0;

        // Binds the irradince map which was generated for this texture cube, remember to generate one before calling this via GenIrradianceMap();
        virtual void BindIrradianceMap(Uint slot) const = 0;

        // Binds the PreFilter map which was generated for this texture cube, remember to generate one before calling this via GenPreFilter();
        virtual void BindPreFilterMap(Uint slot) const = 0;

        virtual bool operator==(const Cubemap& other) const = 0;

        // Calculates the MipMap count
        virtual Uint CalculateMipMapCount(Uint width, Uint height) = 0;

        static Ref<Cubemap> Create(const String& path);
    };
}
