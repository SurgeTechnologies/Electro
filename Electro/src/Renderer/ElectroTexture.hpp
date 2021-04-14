//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"
#include "Core/ElectroBase.hpp"
#include "Renderer/ElectroShader.hpp"
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

        //Reloads the texture, this takes a bool parameter named flip, if that is set to true the texture will be flipped vertically
        virtual void Reload(bool flip = false) = 0;

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

    class TextureCube : public IElectroRef
    {
    public:
        virtual ~TextureCube() = default;

        //Width of a single Texture2D in TextureCube
        virtual Uint GetWidth() const = 0;

        //Height of a single Texture2D in TextureCube
        virtual Uint GetHeight() const = 0;

        //Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        //Returns the *Folder* path
        virtual String GetFolderpath() const = 0;

        //Returns the folder name
        virtual String const GetName() const = 0;

        //Reloads all 6 textures, expensive operation
        virtual void Reload(bool flip = false) = 0;

        //Binds the TextureCube to the pipeline
        virtual void Bind(Uint slot = 0, ShaderDomain domain = ShaderDomain::PIXEL) const = 0;

        //Unbinds the TextureCube from the pipeline, this function may not 100% work for all renderer backends
        virtual void Unbind() const = 0;

        virtual bool operator==(const TextureCube& other) const = 0;

        //Calculates the MipMap count, don't use it if you don't know what MipMaps are, go google for it!
        static Uint CalculateMipMapCount(Uint width, Uint height);

        /*
         Make sure the folder contains the image files named
         Aright, Bleft, Ctop, Dbottom, Efront, Fback,
         where the names represents the 6 sides of a cubemap.
         Yes, the prefix A, B, C, D, E, F are VERY necessary!
        */
        static Ref<TextureCube> Create(const String& folderpath);
    };
}