//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Core/ElectroBase.hpp"
#include "Renderer/Interface/ElectroShader.hpp"
#include <string>
#include <glm/glm.hpp>

namespace Electro
{
    class Texture2D : public IElectroRef
    {
    public:
        virtual ~Texture2D() = default;

        //Returns Width of the loaded Texture2D
        virtual Uint GetWidth() const = 0;

        //Returns Height of the loaded Texture2D
        virtual Uint GetHeight() const = 0;

        //Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        //Returns the filepath from which the texture was loaded
        virtual String GetFilepath() const = 0;

        //Returns the name of the loaded texture, with extension
        virtual String const GetName() const = 0;

        //You can manually set the data for the texture by this function, useful for dynamic/custom textures
        virtual void SetData(void* data, Uint size) = 0;

        //Returns true if the texture is loaded to Electro perfectly
        virtual bool Loaded() = 0;

        //Reloads the texture but it is now flipped
        virtual void ReloadFlipped() = 0;

        //Gets the current texture flip status, returns false if the texture is not flipped
        virtual bool& GetFlipStatus() = 0;

        //Binds the Texture2D to the pipeline
        virtual void Bind(Uint slot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const = 0;

        //Unbinds the Texture2D from the pipeline, this function may not 100% work for all renderer backends
        virtual void Unbind() const = 0;

        virtual bool operator==(const Texture2D& other) const = 0;

        //Calculates the MipMap count, don't use it if you don't know what MipMaps are, go google for it!
        static Uint CalculateMipMapCount(Uint width, Uint height);

        //Create an empty texture, by specifiying its width and height, you can set the data for it via Texture2D::SetData(void* data, Uint size) later
        static Ref<Texture2D> Create(Uint width, Uint height);

        //Loads a texture from the given filepath
        static Ref<Texture2D> Create(const String& path, bool srgb = false, bool flipped = false);
    };

    class Cubemap : public IElectroRef
    {
    public:
        virtual ~Cubemap() = default;

        //Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        //Returns the Path of the HDR texture
        virtual String GetPath() const = 0;

        //Returns the file name
        virtual String const GetName() const = 0;

        //Binds the Cubemap to the pipeline
        virtual void Bind(Uint slot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const = 0;

        //Generates the PreFilter map for the texture cube
        virtual RendererID GenIrradianceMap() = 0;

        //Generates the irradince map for the texture cube
        virtual RendererID GenPreFilter() = 0;

        //Binds the irradince map which was generated for this texture cube, remember to generate one before calling this via GenIrradianceMap();
        virtual void BindIrradianceMap(Uint slot) = 0;

        //Binds the PreFilter map which was generated for this texture cube, remember to generate one before calling this via GenPreFilter();
        virtual void BindPreFilterMap(Uint slot) = 0;

        virtual bool operator==(const Cubemap& other) const = 0;

        //Calculates the MipMap count
        static Uint CalculateMipMapCount(Uint width, Uint height);

        static Ref<Cubemap> Create(const String& path);
    };
}